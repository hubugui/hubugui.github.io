---
layout: post
title: FFMPEG中AVBufferRef                                  # Title of the page
hide_title: false                                   # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"        # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                     # Add bootstrap to the page
tags: [ffmpeg]
published: true
excerpt_separator: <!--more-->
---

<!--more-->
* TOC
{:toc}

# 1. 参考

* [buffer.c](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/buffer.c)
* [buffer.h](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/buffer.h)
* [buffer_internal.h](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/buffer_internal.h)
* [FFMPEG中内存管理](https://hubugui.github.io/2022/08/28/FFMPEG%E4%B8%AD%E5%86%85%E5%AD%98%E7%AE%A1%E7%90%86.html)

# 2. 背景

翻FFMPEG代码时经常看到[AVBufferRef](https://github.com/FFmpeg/FFmpeg/blob/ccfdef79b132bef49f4654266d5d3da8d1deb305/libavutil/buffer.h#L82)，压缩数据[AVPacket](https://github.com/FFmpeg/FFmpeg/blob/ccfdef79b132bef49f4654266d5d3da8d1deb305/libavcodec/packet.h#L350)和未压缩数据[AVFrame](https://github.com/FFmpeg/FFmpeg/blob/ccfdef79b132bef49f4654266d5d3da8d1deb305/libavutil/frame.h#L303)都用它记录实际数据，在不熟悉代码情况下， 要分析数据在生产、使用、消费等各个环节的异常，往往可以通过数据的流转关系，找到她在各个环节分别由谁分配、谁使用、谁释放，从而在脑海中完成一条输入、处理、输出的拼图。

# 3. 结构体定义

## 3.1. **AVBufferRef**定义在libavutil/buffer.h，她引用了下面**AVBuffer**

{% highlight c linenos %}
typedef struct AVBufferRef {
    AVBuffer *buffer;
    uint8_t *data;
    size_t   size;
} AVBufferRef;
{% endhighlight %}

buffer.h中对外API基于该结构并公开定义，而AVBuffer则对外隐藏。AVBufferRef的**data**变量值等于**buffer->data**，size也一样，所以AVBufferRef是一个AVBuffer的**引用**。

## 3.2. **AVBuffer**定义在libavutil/buffer_internal.h

{% highlight c linenos %}
struct AVBuffer {
    uint8_t *data; /**< data described by this buffer */
    buffer_size_t size; /**< size of data in bytes */
    atomic_uint refcount;
    void (*free)(void *opaque, uint8_t *data);
    void *opaque;
    int flags;
    int flags_internal;
};
{% endhighlight %}

AVBuffer中**refcount**是一个原子变量，保障线程安全。FFMPEG中有一组**atomic_**开头的API，提供跨平台的原子操作。

**free**指针标记**data**空间的释放函数，虽然大多数时候内存管理都是标准的**malloc/free**函数，但对于特定场合，可以自行申请**data**空间，比如来自显存，又或者为了更好的回收相关资源，此时就可以传入特定的释放API。

**opaque**是一个**free**时的userdata，**flags**和**flags_internal**标记结构特性。

# 4. API

## 4.1. AVBufferRef *av_buffer_alloc(size)

通过**av_malloc**申请指定长度的内存空间，并通过**av_buffer_create**传入该空间、空间长度、free释放data的API，来创建AVBufferRef并返回。

{% highlight c linenos %}
AVBufferRef *av_buffer_alloc(buffer_size_t size)
{
    AVBufferRef *ret = NULL;
    uint8_t    *data = NULL;

    data = av_malloc(size);
    if (!data)
        return NULL;

    ret = av_buffer_create(data, size, av_buffer_default_free, NULL, 0);
    if (!ret)
        av_freep(&data);

    return ret;
}
{% endhighlight %}

## 4.2. AVBufferRef *av_buffer_create(uint8_t *data, buffer_size_t size,
                              void (*free)(void *opaque, uint8_t *data),
                              void *opaque, int flags)

使用**av_malloc***系列函数创建AVBuffer和AVBufferRef，他们的data、size都等于传入的**data**和**size**变量。flags可指定为AV_BUFFER_FLAG_READONLY表示只读。**函数返回的AVBufferRef引用计数器是1**。

![AVBufferRef、AVBuffer和AVBufferRef->data的三者关系](/assets/img/post/2022-09-19-ffmpeg-avbufferref/ffmpeg-avbufferref.png)

通过av_buffer_create()和av_buffer_alloc()可以看出AVBufferRef、AVBuffer和AVBufferRef->data的三者关系：

1. AVBufferRef/AVBuffer没有强制分配在连续的内存空间。
2. Data可以在内存，也可以在其他设备地址空间。

## 4.3. AVBufferRef *av_buffer_ref(AVBufferRef *buf)

和av_buffer_unref()一起使用，**av_buffer_ref()**分配一个新的AVBufferRef，各个成员变量的值完全等于**buf**，并通过原子操作API给**buf->buffer**记录的AVBuffer引用计数器加1。

## 4.4. void av_buffer_unref(AVBufferRef *buf)

释放buf，并当**buf->buffer**指向的AVBuffer引用计数为1也就是最后一个时，释放**buf->buffer->data**和**buf->buffer**。

## 4.5. int av_buffer_is_writable(const AVBufferRef *buf)

当使用者想要修改buf指向空间时需要使用本API，满足下面条件会返回0，否则返回1：

1. buf->flags等于AV_BUFFER_FLAG_READONLY。
2. buf->buffer的引用计数大于1也就是多个使用者。

## 4.6. int av_buffer_make_writable(AVBufferRef **pbuf)

当**pbuf**不可写时重新申请一个AVBufferRef和其指向的AVBuffer、data空间，将***pbuf->data**数据复制到新的空间，最后让*pbuf等于这个新分配的AVBufferRef，完成一次**克隆**。需要注意的是，只针对默认的内存中分配data和默认的free释放API。