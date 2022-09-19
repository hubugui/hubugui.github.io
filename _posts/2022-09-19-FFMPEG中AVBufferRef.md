---
layout: post
title: FFMPEG中AVBufferRef                                  # Title of the page
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

参考[2022-09-01-FFMPEG编解码器](https://hubugui.github.io/2022/09/01/FFMPEG%E7%BC%96%E8%A7%A3%E7%A0%81%E5%99%A8.html)

# 1. 背景

# 2. 解码

* [buffer.c](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/buffer.c)
* [buffer.h](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/buffer.h)
* [buffer_internal.h](https://github.com/FFmpeg/FFmpeg/blob/n4.4.2/libavutil/buffer_internal.h)

## 2.1. AVBuffer和AVBufferRef

**AVBufferRef**

{% highlight c linenos %}
typedef struct AVBufferRef {
    AVBuffer *buffer;

    /**
     * The data buffer. It is considered writable if and only if
     * this is the only reference to the buffer, in which case
     * av_buffer_is_writable() returns 1.
     */
    uint8_t *data;
    /**
     * Size of data in bytes.
     */
#if FF_API_BUFFER_SIZE_T
    int      size;
#else
    size_t   size;
#endif
} AVBufferRef;
{% endhighlight %}

**AVBuffer**

{% highlight c linenos %}
struct AVBuffer {
    uint8_t *data; /**< data described by this buffer */
    buffer_size_t size; /**< size of data in bytes */

    /**
     *  number of existing AVBufferRef instances referring to this buffer
     */
    atomic_uint refcount;

    /**
     * a callback for freeing the data
     */
    void (*free)(void *opaque, uint8_t *data);

    /**
     * an opaque pointer, to be used by the freeing callback
     */
    void *opaque;

    /**
     * A combination of AV_BUFFER_FLAG_*
     */
    int flags;

    /**
     * A combination of BUFFER_FLAG_*
     */
    int flags_internal;
};
{% endhighlight %}

## 2.2. AVBufferRef *av_buffer_create(uint8_t *data, buffer_size_t size)

{% highlight c linenos %}
AVBufferRef *av_buffer_create(uint8_t *data, buffer_size_t size,
                              void (*free)(void *opaque, uint8_t *data),
                              void *opaque, int flags)
{
    AVBufferRef *ref = NULL;
    AVBuffer    *buf = NULL;

    buf = av_mallocz(sizeof(*buf));
    if (!buf)
        return NULL;

    buf->data     = data;
    buf->size     = size;
    buf->free     = free ? free : av_buffer_default_free;
    buf->opaque   = opaque;

    atomic_init(&buf->refcount, 1);

    buf->flags = flags;

    ref = av_mallocz(sizeof(*ref));
    if (!ref) {
        av_freep(&buf);
        return NULL;
    }

    ref->buffer = buf;
    ref->data   = data;
    ref->size   = size;

    return ref;
}
{% endhighlight %}

## 2.3. AVBufferRef *av_buffer_alloc(size)

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

## 2.4. AVBufferRef *av_buffer_ref(AVBufferRef *buf)

{% highlight c linenos %}
AVBufferRef *av_buffer_ref(AVBufferRef *buf)
{
    AVBufferRef *ret = av_mallocz(sizeof(*ret));

    if (!ret)
        return NULL;

    *ret = *buf;

    atomic_fetch_add_explicit(&buf->buffer->refcount, 1, memory_order_relaxed);

    return ret;
}
{% endhighlight %}

## 2.5. void av_buffer_unref(AVBufferRef *buf)

{% highlight c linenos %}
void av_buffer_unref(AVBufferRef **buf)
{
    if (!buf || !*buf)
        return;

    buffer_replace(buf, NULL);
}
{% endhighlight %}

## 2.6. static void buffer_replace(AVBufferRef **dst, AVBufferRef **src)
{% highlight c linenos %}
static void buffer_replace(AVBufferRef **dst, AVBufferRef **src)
{
    AVBuffer *b;

    b = (*dst)->buffer;

    if (src) {
        **dst = **src;
        av_freep(src);
    } else
        av_freep(dst);

    if (atomic_fetch_sub_explicit(&b->refcount, 1, memory_order_acq_rel) == 1) {
        b->free(b->opaque, b->data);
        av_freep(&b);
    }
}
{% endhighlight %}