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

**湖海风波任澎湃**，一波未平，一波又起，刚处理好[CUDA无效设备](https://hubugui.github.io/2022/08/19/CUDA%E6%97%A0%E6%95%88%E8%AE%BE%E5%A4%87.html)，又发现Windows 7上视频源断线后，foo通过bar增强图像的功能失效，但Windows 10上正常。

![框架](/assets/img/post/2022-08-19-Cuda-invalid/architecture.png "框架")

# 2. 过程

