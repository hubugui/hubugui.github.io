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

ffmpeg解码涉及多个封装完备的源文件，核心数据是**AVFrame**，弄清楚各个子模块围绕着**AVFrame**如何完成准备、加工、使用以及内存复用、生命周期，也就梳理出解码的大致脉络。

# 2. 解码

先看看解码的组成，BTW，找到一个超赞的[国风调色板](http://color.xunmi.cool)，用来画图：

![](/assets/img/post/2022-09-01-ffmpeg-avframe/ffmpeg_decoder.png)

大概是ffmpeg.c或自己开发的相关应用，先调用`avcodec_alloc_context3`

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

# 3. 过程

