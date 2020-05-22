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

{% highlight c linenos %}
#include <stdio.h>
#include <string.h> /* TODO: useless include */

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

上面是功能最简单的C语言代码，控制台输出字符串**Hello World**，短短21行中也有些关隘。

## 1. 多余的`include`

`<string.h>`是不用包含的，因为所有代码中没有使用到该头文件中提供的宏、枚举、结构体、API等。

**凡是不用的就不该出现**也是一条遵循**KISS-Keep It Simple, Stupid!**原则的实践，由于软件是一种规模化、复杂度逐步上升的手工活动，越来越庞大，所以尽量简洁，不给维护者留下多余的负担，如果每个作者都这么做，系统趋向干净，如果纵容，面条代码像小强一样快速繁殖。

## 2.  TAB or Space

代码只有一个维护者、只用一种编辑器，也不需要分享，可以不在乎TAB还是Space，按自己的习惯来。一旦开源或者内部协作，就需要保持约定，避免不同人的不同编辑器及配置，编辑同一份代码时缩进无法对齐、碍眼，降低思维的执行速度，即使你强制修改缩进为你的习惯后提交，他人拉取后也是如此，相互吐槽。

随着互联网发展，出现Google Code、Github、Bitbucket、Gitlab，Web上缩进不对齐更加扎眼。

推荐编辑器中设置TAB为若干个Space，我在Sublime中选择了4个。

## 3. static函数

如果一个函数只在本文件使用，建议在返回值类型前面加入`static`，强调适用范围，避免一会儿自用、一会儿对外暴露的随意行为，外部可以调用哪些函数应有所谋划，而不要沦为随性。

内部被调用的函数只要放到调用者上方，编译器就不会报告未声明的错误，还可以避免每次原型还得同时修改定义和声明这2处位置。

## 4. 常量字符串

如果明确该字符串不应该被某函数修改，应加入`const`前缀，从语法层面昭示内部实现的含义，使得潜在的信息容易识别，降低每次查看代码的成本。

## 5. 函数返回值

一个函数是否有返回值，在设计时应有所明确，基本上推荐都有个`int`类型的返回值，凡是函数都有执行失败的可能，而且失败的原因还有很多种，尽量不要隐藏内部发生了什么，即使今天不需要，也没准改天用得上，如果到处都是这样的不便，会暗示后来者去使用全局变量控制状态，更加恶劣。让本当如此的事情，真实的流露出来，调用者用不用是她的选择，调用者会变化、会有多个调用者，你不提供就是私藏信息。

于是对于调用者而言，也要去判断每个函数的返回值，分别处理，重要的地方还需要打印到日志中来做记录，见过太多不做检查，90%情况都正常，那10%却把开发、测试、运维拖垮。