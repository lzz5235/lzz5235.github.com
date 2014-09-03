---
layout: post
title: "Write Simple Kernel Module"
categories: linux

tags: kernel module
---

编写简单的HelloWorld Kernel Module
===================================
普通的用户态下的C编程，估计很多人都可以熟练编写掌握。但是跨入内核层之后，用户态的函数与头文件统统变了。

首先我们要更改头文件，然后编写代码

<pre><code>
#include <\linux/module.h>
#include <\linux/kernel.h>
#include <\linux/init.h>
 
static int __init lkp_init(void)
{
printk("Hello,world from module!\n");
return 0;
}
 
static void __exit lkp_exit(void)
{
printk("Goodbye,from module!\n");
}
 
module_init(lkp_init);
module_exit(lkp_exit);
 
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lzz");
MODULE_DESCRIPTION("hello");
MODULE_VERSION("prink");
</code></pre>

\__init     \__exit
表示一种初始化的宏，编译器将标__init的所有代码存在特殊的内存段中，初始化结束后就释放这段内存。

Makefile：

<pre><code>
obj-m:= hello_module.o
 
CURRENT_PATH:=$(shell pwd)
 
LINUX_KERNEL:=$(shell uname -r)
 
LINUX_KERNEL_PATH:=/lib/modules/$(LINUX_KERNEL)/build
 
all:#记住make之前是tab，要不编译不过去
make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
 
clean:
make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
</code></pre>

这与我们一般的头文件不同。一般我们在用户态下编写C程序，头文件会放在：/usr/include/下。

而我们模块编程时，它使用的是内核中的头文件。一般在fedora下：cd /lib/modules/$(LINUX_KERNEL)/build。

build下有include/ 然后这个目录下linux/这个子目录中的头文件，因此模块编译的时候会自动在内核中的include/目录下找linux/kernel.h这样的头文件。

其次，printf到printk是一个典型的用户态下编程与内核模块编程的不同。可能我们一开始会比较奇怪，为什么我make成功，加载也成功，但是就是不能显示printk里面的语句呢？我们可以这么想printk就是专门为内核“服务”。它一般输出的语句都在内核的日志文件当中。


<pre><code>
#insmod hello_world.ko
#dmesg | tail //查看
#rmmod hello_world.ko //卸载
</code></pre> 

参考：[http://coolshell.cn/articles/566.html](http://coolshell.cn/articles/566.html)
