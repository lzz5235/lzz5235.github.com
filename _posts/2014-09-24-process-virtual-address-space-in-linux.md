---
layout: post
title: "Process virtual address space in Linux"
categories: read
tags: Process,VADDR
---
Linux进程的虚拟地址空间(笔记)
==============================
进程虚拟空间是个32或64位的“平坦”（独立的连续区间）地址空间（空间的具体大小取决于体系结构）。

为了方便管理，虚拟空间被划分为许多大小可变的(但必须是4096的倍数)内存区域，如果要查看某个进程占用的内存区域，可以使用命令cat /proc/<pid>/maps获得.pid是进程的id号

<pre><code>
[root@localhost user-mode]# cat /proc/16677/maps
00400000-00401000 r-xp 00000000 fd:02 2634667                            /home/lzz/kernel/user-mode/Location
00600000-00601000 r--p 00000000 fd:02 2634667                            /home/lzz/kernel/user-mode/Location
00601000-00602000 rw-p 00001000 fd:02 2634667                            /home/lzz/kernel/user-mode/Location
02556000-02557000 rw-p 00000000 00:00 0                                  [heap]
3fa0000000-3fa0020000 r-xp 00000000 fd:01 2622335                        /usr/lib64/ld-2.16.so
3fa0220000-3fa0221000 r--p 00020000 fd:01 2622335                        /usr/lib64/ld-2.16.so
3fa0221000-3fa0222000 rw-p 00021000 fd:01 2622335                        /usr/lib64/ld-2.16.so
3fa0222000-3fa0223000 rw-p 00000000 00:00 0
3fa0400000-3fa05ad000 r-xp 00000000 fd:01 2622337                        /usr/lib64/libc-2.16.so
3fa05ad000-3fa07ad000 ---p 001ad000 fd:01 2622337                        /usr/lib64/libc-2.16.so
3fa07ad000-3fa07b1000 r--p 001ad000 fd:01 2622337                        /usr/lib64/libc-2.16.so
3fa07b1000-3fa07b3000 rw-p 001b1000 fd:01 2622337                        /usr/lib64/libc-2.16.so
3fa07b3000-3fa07b8000 rw-p 00000000 00:00 0
7fd5a3b10000-7fd5a3b13000 rw-p 00000000 00:00 0
7fd5a3b30000-7fd5a3b32000 rw-p 00000000 00:00 0
7fffb63fb000-7fffb641c000 rw-p 00000000 00:00 0                          [stack]
7fffb6477000-7fffb6479000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
[root@localhost user-mode]#
</code></pre>

每一行信息显示的内容依次为内存区域其实地址-终止地址，访问权限，偏移量，主设备号：次设备号，inode，文件。

从某个内存区域的访问权限上可以大致判断该区域的类型。各个属性符号的意义为：r-read，w-write，x-execute，s-shared，p-private。因此，r-x一般代表程序的代码段，即可读，可执行。rw-可能代表数据段，BSS段和堆栈段等，即可读，可写。堆栈段从行信息的文件名就可以区分；如果某行信息的文件名为空，那么可能是BSS段。另外，上述test进程共享了内核动态库，所以在00441000-00442000行处文件名显示为vdso（Virtual Dynamic Shared Object）。

在Linux内核中对应进程内存区域的数据结构是: vm_area_struct, 内核将每个内存区域作为一个单独的内存对象管理，相应的操作也都一致。采用面向对象方法使VMA结构体可以代表多种类型的内存区域－－比如内存映射文件或进程的用户空间栈等，对这些区域的操作也都不尽相同。

下面是task_struct mm_struct vm_area_strut的关系：

![](/assets/pic/Memory1-1024x768.jpg)

mm_struct结构中，我不理解的字段主要是下面两个，然后我查资料：

start_code, end_code, start_data, end_data：unsigned long类型。进程代码段和数据段的起始地址和终止地址；
start_brk, brk, start_stack：unsigned long类型。分别为堆的起始地址和终止地址，堆栈的起始地址。上文说过，进程的堆栈段是根据需求向下（朝低地址方向）延伸的，因此这里并没有堆栈段的终止地址。

另外之前让我比较困惑的是虚拟地址与物理地址的实际地址问题。

我们都知道虚拟地址中，kernel的地址位于0xC0000000之后，实际上kernel虚拟地址与实际物理地址转换仅仅是进行简单地线性加减法。

page.h头文件中对内核空间中地址映射的说明及定义：

<pre><code>
#define __PAGE_OFFSET (0xC0000000)
……
#define PAGE_OFFSET ((unsigned long)__PAGE_OFFSET)
#define __pa(x) ((unsigned long)(x)-PAGE_OFFSET)
#define __va(x) ((void *)((unsigned long)(x)+PAGE_OFFSET))
</code></pre>

* 给定一个虚地址x，其物理地址为x-PAGE_OFFSET

* 给定一个物理地址x，其虚地址为x+PAGE_OFFSET

<pre><code>
static int __init mmshow_init(void)
{
    printk("mmshow module is working\n");
 
    pagemem = __get_free_page(GFP_KERNEL);
    if(!pagemem)
        goto gfp_fail;
    printk(KERN_INFO "pagemem = 0x%lx\n",pagemem);
 
    kmallocmem = kmalloc(100 * sizeof(char),GFP_KERNEL);
    if(!kmallocmem)
        goto kmalloc_fail;
    printk(KERN_INFO "kmallocmem = 0x%p\n",kmallocmem);
 
    vmallocmem = vmalloc(1000000 * sizeof(char));
    if(!vmallocmem)
        goto vmalloc_fail;
    printk(KERN_INFO "vmallocmem = 0x%p\n",vmallocmem);
 
    return 0;
 
gfp_fail:
    free_page(pagemem);
kmalloc_fail:
    kfree(kmallocmem);
vmalloc_fail:
    vfree(vmallocmem);
 
    return -1;
}
//运行结果：
[ 5542.073900] mmshow module is working
[ 5542.073904] pagemem = 0xf3211000
[ 5542.073907] kmallocmem = 0xd581e700
[ 5542.073983] vmallocmem = 0xf9251000
</code></pre>

在我们之后说的地址空间都是基于逻辑地址地址空间

![](/assets/pic/vm.png)

在kernel中分配内存有三种分配方式：

> 为了满足内核对这种小内存块的需要，Linux系统采用了一种被称为slab分配器的技术。Slab分配器的实现相当复杂，但原理不难，其核心思想就是“存储池”的运用。内存片段（小块内存）被看作对象，当被使用完后，并不直接释放而是被缓存到“存储池”里，留做下次使用，这避免了频繁创建与销毁对象所带来的额外负载。

> kmalloc（）

> vmalloc函数分配内核虚拟内存，该函数不同于kmalloc，它可以分配较Kmalloc大得多的内存空间（可远大于128K，但必须是页大小的倍数），但相比Kmalloc来说,Vmalloc需要更新内核页表，因此分配效率上要低一些。

###kmalloc与vmalloc区别：
kmalloc()分配的内存处于3GB～high_memory之间，这段内核空间与物理内存的映射一一对应，而vmalloc()分配的内存在
VMALLOC_START～4GB之间，这段非连续内存区映射到物理内存也可能是非连续的• vmalloc() 分配的物理地址无需连续，而
kmalloc() 确保页在物理上是连续的。

![](/assets/pic/vm1.png)

在编写内核模块的时候，编写完毕。加载ko的时候

<pre><code>
EXPORT_SYMBOL(符号名);
EXPORT_SYMBOL_GPL(符号名);
</code></pre>

我们可以查看[root@localhost user-mode]# cat /proc/kallsyms | more

导出来的就是在内核模块编程中可用的函数模块。使用 EXPORT_SYMBOL_GPL() 只用于包含 GPL 许可权的模块。 