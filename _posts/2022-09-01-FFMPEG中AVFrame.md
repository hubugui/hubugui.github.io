---
layout: post
title: FFMPEG解码和AVFrame                                  # Title of the page
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

ffmpeg编解码涉及很多源文件，核心数据是**AVFrame**，本文将从视频解码入手，尝试弄清楚代码如何围绕着**AVFrame**完成初始化、编码或解码、使用以及生命周期，就可以梳理出大致脉络。

# 2. 编解码器

ffmpeg提供的编解码器有很多，可通过`ffmpeg -decoders`列出解码器，`ffmpeg -encoders`列出编码器。每个编解码器通过定义`AVCodec`结构体变量，提供自己的名称，类型如音频AVMEDIA_TYPE_AUDIO、视频AVMEDIA_TYPE_VIDEO、字幕AVMEDIA_TYPE_AUDIO，CodecId如AV_CODEC_ID_H264、AV_CODEC_ID_AC3，以及关键的几个回调函数：

* init
* close
* flush
* receive_packet
* decode
* receive_frame

凡是提供了`decode`或`receive_frame`函数指针的`AVCodec`变量，表示其是解码器，否则是编码器。下面是FFMPEG内置的h264解码器(采用CPU解码)的定义，源自libavcodec/h264dec.c，只保留了关键数据：
{% highlight c linenos %}
AVCodec ff_h264_decoder = {
    .name                  = "h264",
    .long_name             = NULL_IF_CONFIG_SMALL("H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_H264,
    .priv_data_size        = sizeof(H264Context),
    .init                  = h264_decode_init,
    .close                 = h264_decode_end,
    .decode                = h264_decode_frame,
    .capabilities          = /*AV_CODEC_CAP_DRAW_HORIZ_BAND |*/ AV_CODEC_CAP_DR1 |
                             AV_CODEC_CAP_DELAY | AV_CODEC_CAP_SLICE_THREADS |
                             AV_CODEC_CAP_FRAME_THREADS,
                               HWACCEL_DXVA2(h264),
    .caps_internal         = FF_CODEC_CAP_INIT_THREADSAFE | FF_CODEC_CAP_EXPORTS_CROPPING |
                             FF_CODEC_CAP_ALLOCATE_PROGRESS | FF_CODEC_CAP_INIT_CLEANUP,
    .flush                 = h264_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(ff_h264_update_thread_context),
    .profiles              = NULL_IF_CONFIG_SMALL(ff_h264_profiles),
    .priv_class            = &h264_class,
};
{% endhighlight %}

FFMPEG也支持Nvidia NvCodec硬件编解码h264、h265、av1、vp9等多种视频格式，通过在libavcodec/cuviddec.c中定义了为每一种视频格式分别定义不同的`AVCodec`变量：

* ff_av1_cuvid_decoder
* ff_h264_cuvid_decoder
* ff_hevc_cuvid_decoder
* ff_vp9_cuvid_decoder

每个变量中的`name`字符串对应`ffmpeg -decoders | grep cuvid`列举出来的解码器名称：

* av1_cuvid
* h264_cuvid
* hevc_cuvid
* vp9_cuvid

同时在libavcodec/allcodecs.c中提供API来访问收集了所有`AVCodec`变量的数组`codec_list`。

# 3. 解码

下面这张图画出大致解码过程中的几个关键源文件及关系，BTW，画图时找到一个超赞的[国风调色板](http://color.xunmi.cool)：

![](/assets/img/post/2022-09-01-ffmpeg-avframe/ffmpeg_decoder.png)

## 3.1. 初始化

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

## 3.2. 解码

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

# 4. AVFrame

`AVFrame`是FFMPEG中解码后的每帧图像或声音的基本单元。

## 4.3. 计算LineSize

[libutils/imgutils.c](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/imgutils.c)提供`av_image_fill_linesizes` API根据给定图像类型、宽，计算对应linesize填写到传入的数组。

其中依赖[libutils/pixdesc.c](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/pixdesc.c)中全局struct数组：`av_pix_fmt_descriptors`，她描述了每种格式各通道数据的布局，比如：

* Planar平面模式，比如YU12(也叫I420)，Y/U/V各在一个独立的连续存储空间。
* Semi-Planar半平面模式，比如NV12，Y和UV分别在独立的连续空间，而UV(CbCr)交错存储，而不是分为三个独立空间。
* Packeted打包模式，比如YUYV422各RGB，每个Y和UV相邻，三个通道都没有独立的连续空间。