---
layout: post
title: 码与茶－Hello World				　					# Title of the page
hide_title: false                                  # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"   # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                   # Add bootstrap to the page
tags: [码与茶]
excerpt_separator: <!--more-->
---

<!--more-->
* TOC
{:toc}

1. [前言](/2020/05/16/%E7%A0%81%E4%B8%8E%E8%8C%B6-%E5%89%8D%E8%A8%80.html)
1. [Hello World](/2020/05/17/%E7%A0%81%E4%B8%8E%E8%8C%B6-Hello-World.html)
1. [模块和接口]()
1. [构建环境]()
1. [视频解码器]()
1. [视频播放器]()
1. [安装包]()
1. [测试]()
1. [CI持续集成]()
1. [CD持续部署]()
1. [结束语]()

{% highlight c linenos %}
#include <stdio.h>
#include <string.h> /* useless include */

static int _bar(const char *foo)
{
    return printf("%s\n", foo);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *foo = "Hello World";

    if (argc > 1)
        foo = argv[1];

    ret = bar(foo);
    printf("execute %s\n", ret > 0 ? "success" : "fail");

    return ret > 0 ? 0 : -1;
}
{% endhighlight %}

上面是功能最简单的Ｃ语言例子，输出字符串"Hello World"到控制台，下面说明短短21行代码中的关隘。

## 1. 多余的包含

`<string.h>`是不用包含的，因为所有代码中没有使用到该头文件中提供的宏、枚举、结构体、API等。

**凡是不用的就不该出现**也是一条遵循**KISS-Keep It Simple, Stupid!**原则的实践，由于软件是一种规模化、复杂度逐步上升的手工活动，只会越来越庞大，所以尽量简洁，不给这份维护者留下多余的负担，如果每个作者都这么做，系统明显趋向干净，如果纵容，面条代码的繁殖速度远超想象。

**行善难而作恶易**

## 2.  TAB or Space

在没有Github、Google Code、SourceForge之前，当代码只有一个维护者，他只使用一种编辑器，可以认为没有缩进符和空格转换的

## 3. static函数

## 4. 常量字符串

## 5. 函数返回值