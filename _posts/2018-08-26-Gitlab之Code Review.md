---
layout: post
title: Gitlab之Code Review                                # Title of the page
hide_title: false                                  # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"   # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                   # Add bootstrap to the page
tags: [gitlab, review]
excerpt_separator: <!--more-->
---

<!--more-->
* TOC
{:toc}

# 1. 关于Gitlab

**Gitlab**是一个类似**Github/Bitbucket**的代码托管软件，有一个显著不同是**Gitlab**提供下载版，可安装到各类**Linux**和**Docket**上，使得公司内部可以搭建私有的**Gitlab**环境，而不用登录公网。

她非常适合那些不愿把源代码提交到公网上如**Github/Bitbucket/Gitlab.com**等平台的组织，对于国内用户而言，可能还有个原因就是访问上述几个国外网站时网络质量不稳定，甚至有时干脆无法访问。

# 2. Code Review步骤
1. **master**分支作为稳定分支，进入**Gitlab**上设置该分支只有**master**角色的用户才能合并其他分支并**push**到远程仓库。通常由**master**角色用户根据软件稳定性来合并。
1. **develop**分支是开发主分支，类似**master**分支，只有**master**角色的用户才能操作。
1. 假设用户**test**开发功能或修复bug时从**develop**创建出新分支，假设任务号为**100**，则新分支命名为**branch_refs100**。待新分支上工作告一段落时比如发生以下情况，可进入下一步：
  * 工作完毕想要合并进**develop**
  * 每积攒200行～500行代码左右。如果1000行才开展**Review**会使得每次占用太多时间。
  * 重要代码
  * 困惑代码
1. 提交最新代码到新分支**branch_refs100**，并**push**到远程仓库。
1. 进入Gitlab的项目主页：
  * 点击`merge_requests`->`New merge request`，在`source branch`->`Select source branch`中选择**branch_refs100**，在`Target branch`中选择**develop**，也就是说源分支是**From**，目标分支是**To**。
  * 点击`Compare branches and continue`，进入新网页后填写`Title`和`Description`，并在`Assignee`中选择邀请**Review**的用户名，`Milestone`和`Labels`最好也有。
  * 点击`Submit merge request`。
1. **test**用户以社交方式通知邀请**Review**的用户，比如**admin**。
1. **admin**用户登录**Gitlab**后在右上角会发现分配给自己的**Review**任务，她进入到该`Merge Request`网页：
  * 点击`Changes`->`Side By Side`，可看到**branch_refs100**和目标分支的区别代码，右侧是提交者的新分支代码。默认网页中仅显示差异部分代码，想要看全部代码需点击`Expand All`。
  * **admin**用户逐行检查差异，一旦发现有改善点，可在右侧该行行号位置点击类似**回复**图标，然后填入`Comment`。
  * 全部Review完成后通知**test**用户。
1. **test**用户访问该次`merge request`网页，点击`Changes`、逐个查看**admin**提交的`comment`：
  * 如果同意则相应修改代码，并点击该`Comment`的`Resolve discussion`，表示解决。
      * `Tips`：点击`Resolve discussion`后面的第2个图标，可快速跳到下一个**admin**留下的`Comment`。
  * 如果不同意，请继续回复，或与**admin**用户口头沟通，令其撤销该`comment`。
  * 如上操作直到所有`Comment`得到确认，提交并push到远程仓库中**branch_refs100**分支。并再次创建merge request，告知**admin**用户再次Review。
1. **admin**用户确认新一轮的merge request后，通过`git`命令手动合并branch_refs100进入`develop`分支，并经过**test**用户同意后删除branch_refs100分支。

至此本次**Code Review**结束。下面摘录Gitlab官方说明。

# 3. [Gitlab官方工作流：Typical flow](https://about.gitlab.com/2017/03/17/demo-mastering-code-review-with-gitlab/)
 
>Excellent code depends on rigorous review. At GitLab, every change is reviewed using this flow:
>
>* A developer merges a change in their feature branch and tests it. When they’re happy they push, and merge a merge request.
>* The developer assigns the merge request to a reviewer, who looks at it and merges line and design level comments as appropriate. When the reviewer is finished, they assign it back to the author.
>* The author addresses the comments. This stage can go around for a while, but once both are happy, one assigns to a final reviewer who can merge.
>* The final reviewer follows the same process again. The author again addresses any comments, either by changing the code or by responding with their own comments.
>* Once the final reviewer is happy and the build is green, they will merge.
>
>To find out more about the importance of code quality, considerations for teams of different sizes and stages, and details on how we develop at GitLab while using GitLab, watch our webcast, "Code Review: A Business Imperative" on demand.

# 4. 后记