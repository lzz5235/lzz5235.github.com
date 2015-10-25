---
layout: post
title: "使用kernel package编译内核img包"
categories: linux
tags: kernel,compiler
---
使用kernel-package编译内核img包
=============================
之前编译内核一般也使用make –> make modules_install –> make install –> update-grub 这一系列步骤，在Debian、Ubuntu机器上可以使用kernel-package来编译安装内核。kernel-package是Debian提供的一个编译Linux内核的一个工具集，安装kernel-package 会同时安装上build-essential、libncurses-dev、linux-source等一系列工具。

首先安装：# apt-get install kernel-package

安装完成后我们可以使用dpkg 查看一下：# dpkg -l

在打印出来的信息中我们可以看到，kernel-package 是 A utility for building Linux kernel related 也就是一个用来构建内核的工具。

<pre><code>
$dpkg -l kernel-package
Desired=Unknown/Install/Remove/Purge/Hold
| Status=Not/Inst/Conf-files/Unpacked/halF-conf/Half-inst/trig-aWait/Trig-pend
|/ Err?=(none)/Reinst-required (Status,Err: uppercase=bad)
||/ Name                Version        Architecture   Description
+++-===================-==============-==============-===========================================
ii  kernel-package      12.036+nmu3    all            A utility for building Linux kernel related
</code></pre>

我们会发现安装完kernel-package之后make等一系列工具也安装了，然后我们还是编译一个内核看一下kernel-package的作用：

<pre><code>
# cd  linux-3.18.21    // 进入你想要编译的内核的解压文件夹
# make menuconfig   // 编译内核，自己选择
# sudo CONCURRENCY_LEVEL=4 make-kpkg --initrd kernel-image  // 这一句就是在使用kernel-package在编译。
</code></pre>

CONCURRENCY_LEVEL=4 是设置多线程（类似于我们make -j4的多线程控制）， make-kpkg就是kernel-package提供的编译工具，–initrd参数是说明在生成的image包里有initrd

……

<pre><code>
dpkg --build      /home//kernel/linux-3.18.21/debian/linux-image-3.18.21
dpkg-deb: building package `linux-image-3.18.21' in `../linux-image-linux-3.18.21.Custom_i386.deb'.
make[2]: Leaving directory `/home/kernel/linux-3.18.21'
make[1]: Leaving directory `/home/kernel/linux-3.18.21'
</code></pre>

从上面的信息可以看出，我们的make-kpkg生成了一个deb文件，其实这就是我们编译好的内核，放在当前内核文件夹的上层目录。到此我们的编译工作结束，我们可以使用dpkg 安装我们刚编译好的目录！如果你的编译的内核对其他机器也适用，你可以拷贝这个deb文件到其他机器上直接安装使用。这个工具使得我们编译内核工作变得更加简单快捷。使用kernel-package编译内核最大的好处是我们可以使用  dpkg -r 删除我们编译的内核。

 

安装我们使用的是 sudo dpkg -i linux-image-3.18.21.Custom_i386.deb
卸载时我们可以直接使用： sudo dpkg -r  linux-image-3.18.21


