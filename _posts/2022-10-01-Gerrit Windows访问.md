---
layout: post
title: Gerrit Windows访问                                  # Title of the page
hide_title: false                                   # Hide the title when displaying the post, but shown in lists of posts
# feature-img: "assets/img/sample.png"              # Add a feature-image to the post
# thumbnail: "assets/img/stb_background.png"        # Add a thumbnail image on blog view
# color: rgb(80,140,22)                             # Add the specified color as feature image, and change link colors in post
bootstrap: true                                     # Add bootstrap to the page
tags: [Gerrit]
published: false
excerpt_separator: <!--more-->
---

<!--more-->
## Gerrit账户

确保你的Gerrit账户可以正常登录，并且有某些Project的访问权限：
1. 假设你的账户是**foo**。
1. Gerrit地址是**10.0.0.18**，HTTP服务端口30000，SSH服务端口30001。
1. 你拥有bar项目的仓库访问权限：**ssh://foo@10.0.0.18:30000/bar**。

## 产生公钥和私钥

1. 用管理员权限打开**git bash**。
1. 输入`ssh-keygen -t rsa -C foo`，随后的若干项提示输入时，均输入回车键，一路向下直到完成。
1. 复制`~/.ssh/id_rsa.pub`文件全部内容。
1. 登录Gerrit，点击左侧菜单栏的**SSH Public Key**，点击**Add Key**会展开一个输入框，黏贴复制内容，然后点击**Add**。
1. 随后网页下方会出现许多**Server Host Key**的内容项，每一条都对应不同的加密算法，找到**ssh-rsa**字样所在的行，点击其右侧的复制小图标，当你鼠标放上去时会提示**Copy to clipboard**。
1. 编辑`~/.ssh/known_hosts`，黏贴复制内容，保存。
1. 编辑`/etc/ssh/ssh_config`，把如下内容复制到末尾，并保存：
```
Host 10.0.0.18
	HostkeyAlgorithms +ssh-rsa
	PubkeyAcceptedAlgorithms +ssh-rsa
```

## 验证

1. 验证SSH访问权限：`ssh -p 30001 foo@10.0.0.18`，正常会提示如下开头的内容：** ****    Welcome to Gerrit Code Review    **** **。
1. 如上成功后可克隆代码：`git clone ssh://foo@10.0.0.18:30000/bar`