---
layout: post
title: Nomad & Consul的服务健康检查                 # Title of the page
hide_title: false                                  # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"   # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                   # Add bootstrap to the page
tags: [nomad, consul]
excerpt_separator: <!--more-->
---

<!--more-->
* TOC
{:toc}

一直没有透彻理解Nomad中服务的健康检查、重启以及重新调度的定时机理，翻翻v0.11.2版本代码，很简洁。

以下面job为例：

{% highlight c linenos %}
group "foo" {
    restart {
        interval = "30m"
        attempts = 3
        delay    = "15s"
        mode     = "fail"
    }
    
    reschedule {
        delay          = "30s"
        delay_function = "exponential"
        max_delay      = "1h"
        unlimited      = true
    }

    task "bar" {
        service {
            check {
                address_mode = "host"
                type     = "http"
                port     = "PORT"
                path     = "/"
                interval = "5s"
                timeout  = "2s"
                check_restart {
                    limit = 5
                    grace = "300s"
                    ignore_warnings = false
                }
            }
        }
    }
}
{% endhighlight %}

#### 1. 服务不健康

每间隔900毫秒，调用函数[command/agent/consul/check_watcher.go的函数apply()](https://github.com/hashicorp/nomad/blob/v0.11.2/command/agent/consul/check_watcher.go#L80)判断是否需要重启：

大概判断的C伪代码逻辑如下：

{% highlight c linenos %}
// 返回true表示该服务应立即重启，false表示需继续观察
boolean apply(service, now)
{
    if (service.isHealthy()) {
        service.unhealthyState = 0;
        return false;
    }

    // 服务第一次启动后的总运行时间还短，小于grace配置。这是为避免那些慢启动的服务，还未做好准备就被视为不健康而重启，陷入死循环
    if (service.runningTime < service.check.check_restart.grace)
        return false;

    if (service.unhealthyState == 0)
        service.unhealthyState = now;

    // 以上面配置为例，连续5 * (5 - 1) = 20秒不健康后被重启
    restartTime = service.unhealthyState + service.check.interval * (service.check.check_restart.limit - 1);
    return restartTime <= now ? true : false;
}
{% endhighlight %}

#### 2. 任务重启

通过[client/allocrunner/taskrunner/task_runner.go的函数shouldRestart()](https://github.com/hashicorp/nomad/blob/v0.11.2/client/allocrunner/taskrunner/task_runner.go#L689)调用[client/allocrunner/taskrunner/restarts/restarts.go的函数GetState()](https://github.com/hashicorp/nomad/blob/v0.11.2/client/allocrunner/taskrunner/restarts/restarts.go#L133)，伪代码逻辑如下：

{% highlight c linenos %}

// 返回true表示该服务应立即重启，false表示需继续观察
int GetState(restartTracker, now)
{
    // 任务正常则退出
    if (!restartTracker.failure) {
        return "", 0;
    }

    // 如果跟踪器本轮的运行时间已经超过job中配置
    if (now >= restartTracker.startTime + restartTracker.restart.Interval)) {
        restartTracker.restartCount = 0;
        restartTracker.startTime = now;
    }
    restartTracker.restartCount++;

    // 重启次数已经超过job中配置
    if（restartTracker.restartCount > restartTracker.restart.Attempts) {
        if (restartTracker.restart.Attempts == "fail") {
            // 不用重启了，等待重新调度
            return structs.TaskNotRestarting, 0;
        } else {
            // 重启吧，在getDelay()时间点之后
            return structs.TaskRestarting, restartTracker.getDelay()
        }        
    }
}

{% endhighlight %}

#### 3. 任务重新调度

**未完待续**