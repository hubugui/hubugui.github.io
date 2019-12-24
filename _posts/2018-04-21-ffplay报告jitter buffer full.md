---
layout: post
title: 机顶盒软件的坑                                # Title of the page
hide_title: false                                  # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"   # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                   # Add bootstrap to the page
tags: [机顶盒]
excerpt_separator: <!--more-->
---



# 现象

ffplay播放RTSP时控制台频繁打印：

```
jitter buffer full
RTP: missed 88 packets
```

同时画面卡顿。

# 分析

找到FFMPEG 3.3.2源码[FFmpeg\libavformat\rtpdec.c](https://github.com/FFmpeg/FFmpeg/blob/n3.3.2/libavformat/rtpdec.c)的`rtp_parse_one_packet`函数代码：

```
if ((s->seq == 0 && !s->queue) || s->queue_size <= 1) {
    /* First packet, or no reordering */
    return rtp_parse_packet_internal(s, pkt, buf, len);
} else {
    uint16_t seq = AV_RB16(buf + 2);
    int16_t diff = seq - s->seq;

    if (diff < 0) {
        /* Packet older than the previously emitted one, drop */
        av_log(s->ic, AV_LOG_WARNING,
               "RTP: dropping old packet received too late\n");
        return -1;
    } else if (diff <= 1) {
        /* Correct packet */
        rv = rtp_parse_packet_internal(s, pkt, buf, len);
        return rv;
    } else {
        /* Still missing some packet, enqueue this one. */
        rv = enqueue_packet(s, buf, len);
        if (rv < 0)
            return rv;
        *bufptr = NULL;

        /* Return the first enqueued packet if the queue is full,
         * even if we're missing something */
        if (s->queue_len >= s->queue_size) {
            av_log(s->ic, AV_LOG_WARNING, "jitter buffer full\n");
            return rtp_parse_queued_packet(s, pkt);
        }
        return -1;
    }
}
```

看上去是`s->queue_len >= s->queue_size`时就输出`"jitter buffer full"`，前者应该是变化的，后者是通过[rtsp.c](https://github.com/FFmpeg/FFmpeg/blob/n3.3.2/libavformat/rtsp.c)中的`ff_rtsp_open_transport_ctx() -> ff_rtp_parse_open()`来设置的，`ff_rtsp_open_transport_ctx()`代码如下：

```
RTSPState *rt = s->priv_data;
AVStream *st = NULL;

int reordering_queue_size = rt->reordering_queue_size;
if (reordering_queue_size < 0) {
    if (rt->lower_transport == RTSP_LOWER_TRANSPORT_TCP || !s->max_delay)
        reordering_queue_size = 0;
    else
        reordering_queue_size = RTP_REORDER_QUEUE_DEFAULT_SIZE;
}

/* open the RTP context */
if (rtsp_st->stream_index >= 0)
    st = s->streams[rtsp_st->stream_index];
if (!st)
    s->ctx_flags |= AVFMTCTX_NOHEADER;

if (CONFIG_RTSP_MUXER && s->oformat && st) {
    int ret = ff_rtp_chain_mux_open((AVFormatContext **)&rtsp_st->transport_priv,
                                    s, st, rtsp_st->rtp_handle,
                                    RTSP_TCP_MAX_PACKET_SIZE,
                                    rtsp_st->stream_index);

    /* Ownership of rtp_handle is passed to the rtp mux context */
    rtsp_st->rtp_handle = NULL;
    if (ret < 0)
        return ret;
    st->time_base = ((AVFormatContext*)rtsp_st->transport_priv)->streams[0]->time_base;
} else if (rt->transport == RTSP_TRANSPORT_RAW) {
    return 0; // Don't need to open any parser here
} else if (CONFIG_RTPDEC && rt->transport == RTSP_TRANSPORT_RDT && st)
    rtsp_st->transport_priv = ff_rdt_parse_open(s, st->index,
                                        rtsp_st->dynamic_protocol_context,
                                        rtsp_st->dynamic_handler);
else if (CONFIG_RTPDEC)
    rtsp_st->transport_priv = ff_rtp_parse_open(s, st,
                                     rtsp_st->sdp_payload_type,
                                     reordering_queue_size);
```

[rtsp.c](https://github.com/FFmpeg/FFmpeg/blob/n3.3.2/libavformat/rtsp.c)开头有下面代码，表明上面第3行的`rt->reordering_queue_size`可通过命令行设置，并且初始值等于`-1`：

```
#define COMMON_OPTS() \
    { "reorder_queue_size", "set number of packets to buffer for handling of reordered packets", OFFSET(reordering_queue_size), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, DEC }, \
    { "buffer_size",        "Underlying protocol send/receive buffer size",                  OFFSET(buffer_size),           AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, DEC|ENC } \
```

再结合`ff_rtsp_open_transport_ctx()`的4-9行，说明如果使用`rtsp_transport tcp`来指明TCP传输并且没有设置`reordering_queue_size`，那么调用`ff_rtp_parse_open()`传递的`queue_size`等等于0，接下来开头`rtp_parse_one_packet()`函数的第1行，判断出`s->queue_size <= 1`就不会打印"jitter buffer full"。


再参考下下面代码：

```
static int has_next_packet(RTPDemuxContext *s)
{
    return s->queue && s->queue->seq == (uint16_t) (s->seq + 1);
}

int64_t ff_rtp_queued_packet_time(RTPDemuxContext *s)
{
    return s->queue ? s->queue->recvtime : 0;
}

static int rtp_parse_queued_packet(RTPDemuxContext *s, AVPacket *pkt)
{
    int rv;
    RTPPacket *next;

    if (s->queue_len <= 0)
        return -1;

    if (!has_next_packet(s))
        av_log(s->ic, AV_LOG_WARNING,
                "RTP: missed %d packets\n", s->queue->seq - s->seq - 1);

    /* Parse the first packet in the queue, and dequeue it */
    rv   = rtp_parse_packet_internal(s, pkt, s->queue->buf, s->queue->len);
    next = s->queue->next;
    av_freep(&s->queue->buf);
    av_freep(&s->queue);
    s->queue = next;
    s->queue_len--;
    return rv;
}
```

# 原因及对策

所以发生该问题时，原因有两种：

#### 1. 采用UDP传输，默认的或手动设置的`reordering_queue_size`值过小。

解决办法：继续增大`reordering_queue_size`。

#### 2. 采用TCP传输，手动设置的`reordering_queue_size`值过小。

解决办法：不设置`reordering_queue_size`。

# 查看`reordering_queue_size`大小

#### 1. UDP传输时`reordering_queue_size`

```
ffmpeg -rtsp_transport udp -i rtsp://192.168.5.120:8554/desktop -loglevel verbose
```

可看到

```
[rtsp @ 0000000000537fe0] setting jitter buffer size to 500
```

其实就是[rtsp.c](https://github.com/FFmpeg/FFmpeg/blob/n3.3.2/libavformat/rtsp.c)中的`RTP_REORDER_QUEUE_DEFAULT_SIZE`

#### 2. TCP传输时`reordering_queue_size`

```
ffmpeg -rtsp_transport tcp -i rtsp://192.168.5.120:8554/desktop -loglevel verbose
```

可看到
```
[rtsp @ 0000000000647fe0] setting jitter buffer size to 0
```

# `reordering_queue_size`的意义

根据[rtpdec.c](https://github.com/FFmpeg/FFmpeg/blob/n3.3.2/libavformat/rtpdec.c)的`rtp_parse_one_packet()`，大概UDP传输RTP时会出现乱序，比如上一个RTP序号是88，新接收到RTP包序号100，此时ffplay将继续等待序号为89的RTP包而不解析序号100的包，如果现在积攒了500个RTP包，但89号包还没有到达，则打印该警告，并且解析离88最近的那个RTP包。