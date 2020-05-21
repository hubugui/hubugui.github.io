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

<!--more-->
* TOC
{:toc}

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

勿用解释，上面是功能最简单的C语言代码，输出字符串**Hello World**到控制台，这短短21行中也有些关隘。

## 1. 多余的`include`

`<string.h>`是不用包含的，因为所有代码中没有使用到该头文件中提供的宏、枚举、结构体、API等。

**凡是不用的就不该出现**也是一条遵循**KISS-Keep It Simple, Stupid!**原则的实践，由于软件是一种规模化、复杂度逐步上升的手工活动，只会越来越庞大，所以尽量简洁，不给这份维护者留下多余的负担，如果每个作者都这么做，系统明显趋向干净，如果纵容，面条代码的繁殖速度远超想象。

## 2.  TAB or Space

代码只有一个维护者、只用一种编辑器，也不需要分享，可以不在乎缩进符或空格，怎么舒服怎么来。一旦开源或者伙伴们协作，就需要在不同编辑器中保持同样的缩进，避免不同人的不同编辑器及其配置，导致缩进碍眼，影响思维的正常流转。而随着互联网发展进入Web在线浏览代码，比如Google Code、Github、Bitbucket、Gitlab，你用编辑器配置TAB占用4个空格，浏览器可能显示出过宽或过窄。

所以推荐在编辑器中设置TAB为若干个Space，比如我在Sublime中选择是4个。

## 3. static函数

如果一个函数只在本文件使用，建议在返回值类型前面加入`static`，强调适用范围，避免一会儿自用、一会儿分享的随意行为，哪些功能API需要外部调用应该是有所谋划，而不要沦为想到哪写到哪。

而且被调用的函数只要放到放到调用者的上面，编译器就不会报告未定义的错误，那些需要提升定义原型的写法，每次增减参数都需要修改2处，麻烦。

## 4. 常量字符串

如果明确该字符串不应该被某函数修改，加入`const`前缀，多从语法层面显示出内部实现的含义，使得潜在的信息容易被识别，降低每次查看代码的成本。

## 5. 函数返回值

一个函数是否要有返回值，在设计她时应有所明确，也推荐基本都有个`int`类型的返回值，凡是函数都有执行失败的可能，而且失败的原因还有很多种，尽量不要把内部发生了什么隐藏起来，因为即使今天不需要，也没准改天用得上，如果到处都是这样的不方便，会暗示后来者去使用全局变量控制状态，更加恶劣。让本就如此的事情，真实的暴露出来，调用者用不用是她的选择，你不提供就携裹了信息。

所以对于调用者而言，也要去判断每个函数的返回值，分别处理，重要的地方还需要打印到日志中来做记录，见过太多不做检查，90%情况都正常，那10%可以把团队拖垮。