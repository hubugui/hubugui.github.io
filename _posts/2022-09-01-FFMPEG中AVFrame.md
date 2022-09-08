---
layout: post
title: FFMPEG中AVFrame                                  # Title of the page
hide_title: false                                   # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"        # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                     # Add bootstrap to the page
tags: [ffmpeg]
excerpt_separator: <!--more-->
---

<!--more-->
* TOC
{:toc}

# 1. 背景

参考[2022-09-01-FFMPEG编解码器]()

ffmpeg编解码涉及很多源文件，核心数据是**AVFrame**，本文将从视频解码入手，尝试弄清楚代码如何围绕着**AVFrame**完成初始化、编码或解码、使用以及生命周期，就可以梳理出大致脉络。

# 2. 解码

下面这张图画出大致解码过程中的几个关键源文件及关系，BTW，画图时找到一个超赞的[国风调色板](http://color.xunmi.cool)：

![](/assets/img/post/2022-09-01-ffmpeg-avframe/ffmpeg_decoder.png)

## 2.1. 初始化

首先ffmpeg.c/ffplay.c/ffprobe.c或是你基于ffmpeg库开发的相关xxx.c，都要先调用`avcodec_alloc_context3`拿到一个`AVCodecContext *`的返回值`avctx`，顾名思义是一个音视频编解码器的上下文。接下来为将要解码的数据流选择解码器，也就是一个`AVCodec`实例`codec`。涉及到的API有：

* allcodecs.c中的`avcodec_find_decoder()`
* allcodecs.c中的`avcodec_find_decoder_by_name()`

随后通过`av_dict_set()`给`codec`准备参数，存储到`AVDictionary`类型的字典变量`opt`。最后调用`avcodec_open2()`把`avctx`、`codec`、`opt`捆绑到一起。

**代码示例-裁剪自ffplay.c**

{% highlight c linenos %}
avctx = avcodec_alloc_context3(NULL);
ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
codec = avcodec_find_decoder(avctx->codec_id);

switch(avctx->codec_type){
    case AVMEDIA_TYPE_AUDIO   : is->last_audio_stream    = stream_index; forced_codec_name =    audio_codec_name; break;
    case AVMEDIA_TYPE_SUBTITLE: is->last_subtitle_stream = stream_index; forced_codec_name = subtitle_codec_name; break;
    case AVMEDIA_TYPE_VIDEO   : is->last_video_stream    = stream_index; forced_codec_name =    video_codec_name; break;
}
if (forced_codec_name)
    codec = avcodec_find_decoder_by_name(forced_codec_name);

avctx->codec_id = codec->id;
opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
if (!av_dict_get(opts, "threads", NULL, 0))
    av_dict_set(&opts, "threads", "auto", 0);
if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
    goto fail;
}
{% endhighlight %}

## 2.2. 解码

调用`avcodec_send_packet()`给`avctx`传入新的压缩包，同时调用`avcodec_receive_frame()`读取原始图像。

**代码示例**
{% highlight c linenos %}
static int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt)
{
    int ret;

    *got_frame = 0;

    if (pkt) {
        ret = avcodec_send_packet(avctx, pkt);
        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.
        if (ret < 0 && ret != AVERROR_EOF)
            return ret;
    }

    ret = avcodec_receive_frame(avctx, frame);
    if (ret < 0 && ret != AVERROR(EAGAIN))
        return ret;
    if (ret >= 0)
        *got_frame = 1;

    return 0;
}
{% endhighlight %}

# 3. AVFrame

`AVFrame`是FFMPEG中解码后的每帧图像或声音的基本单元。

## 3.3. 计算LineSize

[libutils/imgutils.c](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/imgutils.c)提供`av_image_fill_linesizes` API根据给定图像类型、宽，计算对应linesize填写到传入的数组。

其中依赖[libutils/pixdesc.c](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/pixdesc.c)中全局struct数组：`av_pix_fmt_descriptors`，她描述了每种格式各通道数据的布局，比如：

* Planar平面模式，比如YU12(也叫I420)，Y/U/V各在一个独立的连续存储空间。
* Semi-Planar半平面模式，比如NV12，Y和UV分别在独立的连续空间，而UV(CbCr)交错存储，而不是分为三个独立空间。
* Packeted打包模式，比如YUYV422各RGB，每个Y和UV相邻，三个通道都没有独立的连续空间。