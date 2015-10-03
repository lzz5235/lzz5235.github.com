---
layout: post
title: "为修改的kernel生成patch"
categories: linux
tags: patch,kernel
---
为修改的kernel生成patch
=======================
最近在为之前修改的内核打patch，平时对于patch只是停留在用用的地步，比如我们这里有一个palloc-3.18.21.patch,打补丁，我们只需要进入到linux-3.18.21根目录下，然后使用命令：

<pre><code>
$patch -p1 less palloc-3.18.21.patch
</code></pre>

如果我们想撤销，那么我们只需要下面的命令：

<pre><code>
$patch -R -p1 less palloc-3.18.21.patch
</code></pre>

那么patch -p0 -p1 是什么意思呢？

* patch -p0 (p指的是路径，后面的数字表示去掉路径的第几部分。0，表示不去掉，为全路径)
* patch -p1 (p后面的数字1，表示去掉前第一个路径)

比如我们修改了内核的某个文件，我们可以使用diff生成单个文件的patch，可以使用下面的命令

<pre><code>
$diff -urN arch/x86/include/ ../linux-3.18.21/arch/x86/include/ > patch
</code></pre>

比如我们我们除了修改源码，还修改了kernel的头文件，那么如何将多个补丁放在一个patch文件中呢？

##方法一：

<pre><code>
$echo patch1 >> patch2
</code></pre>

##方法二：

<pre><code>
$diff -uNr srcDir dstDir > all.patch
</code></pre>

[http://bbs.chinaunix.net/thread-2230170-1-1.html](http://bbs.chinaunix.net/thread-2230170-1-1.html)