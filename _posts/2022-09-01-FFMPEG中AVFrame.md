---
layout: post
title: FFMPEG中AVFrame                                     # Title of the page
hide_title: false                                   # Hide the title when displaying the post, but shown in lists of posts
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

先看看解码的组成：

![](/assets/img/post/2022-09-01-ffmpeg-avframe.png)

大概是ffmpeg.c或自己开发的相关应用，先调用`avcodec_alloc_context3`

# 3. 过程

