---
layout: post
title: "Create Git Server"
categories: others
tags: git

---
创建Git服务器
=========

之前一直都是使用git 客户端，非常的方便，可以通过分支并行开发，加快项目开发速度。

git的使用我就不赘述了，这里我记录一下如何创建git 服务端。


如果我们规定项目必须有固定的几个人开发，这样的话

- Step1：

在服务端，首先安装git和ssh。

- Step2：

创建一个git的user和group。我在fedora下，默认是一个用户对应一个用户组，也就是说当我使用useradd git时，默认创建了两者。

然后在切换到git用户，创建respositories,比如我们取名project.git。

###记得要把git文件下的权限改称770，否则客户端无法clone！

<pre><code>
cd project.git
git init --bare
[git@localhost project.git]$ ls
branches config description HEAD hooks info objects refs
</code></pre>

- Step3:

使用useradd命令创建,useradd -N -g git XXX  这样就把XXX 加到git组的。 

切换到用户电脑，只要输入git clone XXX@IP:/home/git/project.git 就可以了。

如果不想输入/home/git/project.git

[XXX@localhost]$git remote addorigin  XXX@IP:/home/git/project.git

之后就很方便了！


