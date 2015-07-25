---
layout: post
title: "memtest86+与BadRAM使用"
categories: linux
tags: memory,BadRAM
---
memtest86+与BadRAM使用
============
一般情况下，DRAM的损坏是永久性的损坏，这个时候我们有三种方式解决这个问题：

* 买新的内存条
* 在kernel启动时加入mem参数，限制当前mem的使用，比如当前内存2GB，在800M地方，内存存在永久故障区域，那么我们可以在kernel的启动参数加入mem=780M，这样就可以限制当前内核分配800M的内存区域而触发MCE。但是这个也有一个明显的缺点：因为这个坏点导致大部分内存无法使用
* 在kernel启动加入badram 0x01000000,0xfffffffc即可。

我们这里使用第三种方式来绕过当前的内存坏块，而达到省钱的目的！badram 第一个参数 是出错的物理基地址，第二个参数是mask，来用标示这个掩码。当我们使用memtest86+测试出当前的出错内存物理地址，然后将这个错误地址加入到kernel 参数中即可。

其中memtest86+是一款离线内存检测工具，可以检测内存中的坏页。

下面我们来实验一下，比如当前系统没有出现内存坏块，那么使用iomem参看当前物理内存地址的分布:

<pre><code>
00000000-00000fff : reserved
00001000-0009f3ff : System RAM
0009f400-0009ffff : reserved
000a0000-000bffff : PCI Bus 0000:00
000c0000-000dffff : PCI Bus 0000:00
  000c0000-000c7fff : Video ROM
  000cc000-000cd7ff : Adapter ROM
000e0000-000effff : pnp 00:08
000f0000-000fffff : reserved
  000f0000-000fffff : System ROM
00100000-7f6dffff : System RAM
  01000000-017b9cc4 : Kernel code
  017b9cc5-01d1e8ff : Kernel data
  01e86000-01fc8fff : Kernel bss
....
</code></pre>

当我们在grub中加入badram 0x01000000,0xfffffffc参数，也就意味着Kernel代码区的地址出现错误，而且错误大小是2bit，这个时候我们重新启动，再查看当前系统物理内存分布，我们会发现：

<pre><code>
01000000-010003ff : RAM buffer
01000400-7f6dffff : System RAM
  02000000-027b9cc4 : Kernel code
  027b9cc5-02d1e8ff : Kernel data
  </code></pre>
  
kernel 的代码段避过了问题内存区域，我们查看__pa(x)最终调用__phys_addr_nodebug，而其中phys_base则是在real_mode下面被调用。

<pre><code>
static inline unsigned long __phys_addr_nodebug(unsigned long x)
{
    unsigned long y = x - __START_KERNEL_map;
 
    /* use the carry flag to determine if x was  __START_KERNEL_map */
    x = y + ((x > y) ? phys_base : (__START_KERNEL_map - PAGE_OFFSET));
 
    return x;
}
</code></pre>

![](/assets/pic/22996709_13505212884HWu.png)

Real-mode code 在X+0x8000开始，X就是badram传入的offset！