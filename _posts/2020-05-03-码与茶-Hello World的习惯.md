---
layout: post
title: 码与茶－Hello World的习惯　					# Title of the page
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

软件用户分为几种类型：

1. 使用者，通常字面意义上的用户。
2. 电脑，软件的运行环境。
3. 运维者，软件遇到故障、或需要分析某个现象的伙伴。
4. 作者/维护者，三个月后的自己、接手软件或帮你找bug的伙伴。

开发者绝大部分的时间精力花在满足使用者和电脑，也就是功能和性能，在保障进度计划的前提下，这2项若能达到70分已属不易，所以常常冷落了运维者和维护者，她们的要求不像功能那样容易度量，现在讲解相关内容的文字有很多，比如**坏味道**、**重构**，阅读提炼之后的做什么、不做什么的技法是模仿入门，科技和人文的环境日日渐进，若无法领悟到技巧之下的自然规律，便难做到举一反三，我们要始终沿着待解决问题的朴素方向前进，延缓软件腐化的速度，让她长在南方，移植华北，加一层壳子，东北亦能生根。

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
    const char *foo = "Hello World";

    ret = _bar(foo);

    printf("execute %s\n", ret > 0 ? "success" : "fail");

    return ret > 0 ? 0 : -1;
}
{% endhighlight %}

上面简短的C语言代码通过调用内部函数，在控制台输出**Hello World**。

## 1. 多余的`include`

`#include <string.h>`包含一个标准库的头文件，应该删除，因为代码中没有使用她，反而增加了编译工作。实际开发活动中冗余包含库文件也还好，她们极少改动，但那些自己编写的头文件频繁更新，导致构建时浪费时间重新编译所有包含她的文件。

或许你会说我的工程文件少影响不大，但我们现在谈论的就是习惯，高质量不是做完几件事就可以达成，甚至标准也随着编程语言、技术演化的发生改变，

**不用的就不该出现**也是一条遵循**KISS-Keep It Simple, Stupid!**原则的实践，由于软件是一种规模化、复杂度逐步上升的手工活动，越来越庞大，所以尽量简洁，不给维护者留下多余的负担，如果每个作者都这么做，系统趋向干净，如果纵容，面条代码像小强一样快速侵入频繁修改的函数。

## 2.  TAB or Space

代码只有一个维护者、只用一种编辑器，也不需要分享，可以不在乎TAB还是Space，按自己习惯来。一旦开源或者内部协作，就需要保持约定，避免不同人的不同编辑器及配置，编辑同一份代码时缩进参次不齐、碍眼，降低阅读时代码在脑里的执行速度，即使按你的习惯强制修改缩进、提交，他人拉取后也是如此，进而相互吐槽。

随着互联网发展，出现Google Code、Github、Bitbucket、Gitlab等Web上代码阅读器，Web上缩进不对齐更加扎眼。

推荐编辑器中设置TAB为4个Space。

## 3. static函数

如果一个函数只在本文件使用，建议在返回值类型前面加入`static`，强调适用范围，避免一会儿自用、一会儿对外暴露的随意行为，外部可以调用哪些函数应有所谋划，而不要随性。

内部被调用的函数只要放到调用者上方，编译器就不会报告未声明的错误，避免原型变化时同时修改定义和声明这2处位置。

## 4. 常量字符串

如果明确该字符串不应该被某函数修改，应加入`const`前缀，从语法层面昭示内部实现的含义，使得潜在的信息容易识别，降低每次查看代码的成本。

## 5. 函数返回值

一个函数是否有返回值，在设计时应有所明确，基本上推荐都有个`int`类型的返回值，凡是函数都有执行失败的可能，而且失败的原因还有很多种，尽量不要隐藏内部发生了什么，即使今天不需要，也没准改天用得上，如果到处都是这样的不便，会暗示后来者去使用全局变量控制状态，更加恶劣。让本当如此的事情，真实的流露出来，调用者用不用是她的选择，调用者会变化、会有多个调用者，你不提供就是私藏信息。

于是对于调用者而言，也要去判断每个函数的返回值，分别处理，重要的地方还需要打印到日志中来做记录，见过太多不做检查，90%情况都正常，那10%却把开发、测试、运维拖垮。