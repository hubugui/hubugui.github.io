---
layout: post
title: mingw64环境socket设置非阻塞失败                               # Title of the page
hide_title: false                                   # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"        # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                     # Add bootstrap to the page
published: true
tags: [开发, mingw64]
excerpt_separator: <!--more-->
---

<!--more-->
* TOC
{:toc}

使用mingw64(gcc版本11.3.0)编译一个64位软件，如下设置socket为非阻塞模式会失败：

{% highlight c linenos %}
u_long nb = 1;
ret = ioctlsocket(fd, FIONBIO, &nb);
if (ret) {
    error = WSAGetLastError();
}
{% endhighlight %}

ret等于SOCKET_ERROR，error等于WSAEOPNOTSUPP(10045)，chatgpt也给不出有效信息。找到[0x8004667E is what you get](https://stackoverflow.com/a/16185001/4065645)，试试真好了，看着是mingw中宏定义`FIONBIO`和原生的不同。

1. Windows原生**winsock2.h**定义：

{% highlight c linenos %}
#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                        /* 0x20000000 distinguishes new &
                                           old ioctl's */
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))
#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define FIONBIO         _IOW('f', 126, u_long) /* set/clear non-blocking i/o */
{% endhighlight %}


2. mingw中**winsock2.h**定义：

{% highlight c linenos %}
#define IOCPARM_MASK 0x7f
#define IOC_VOID 0x20000000
#define IOC_OUT 0x40000000
#define IOC_IN 0x80000000
#define IOC_INOUT (IOC_IN|IOC_OUT)

#define _IO(x,y) (IOC_VOID|((x)<<8)|(y))
#define _IOR(x,y,t) (IOC_OUT|(((__LONG32)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define _IOW(x,y,t) (IOC_IN|(((__LONG32)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define FIONBIO _IOW('f',126,u_long)
{% endhighlight %}

两者看上去存在`__LONG32`和`long`的不同，实测发现mingw64环境下`FIONBIO`等于**0x8008667E**，而不是期望的**0x8004667E**，这是由于mingw64中`u_long`是8个字节导致。