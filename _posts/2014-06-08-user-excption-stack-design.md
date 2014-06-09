---
layout: post
title: User Excption Stack Design
category: jos
tags: [kernel]
---
用户空间的异常栈设计
======================
这个坑想了很久，但是一直没时间填，今天抽出一个周六，把之前的草稿完成！

我们知道在linux中如果用户程序发生错误，kernel会终止程序运行，并发出segment fault错误。
这种实现其实就是在用户空间实现了异常栈的操作。

下面我来说一下JOS的用户空间的异常栈设计。


首先明确几个概念：

在明确概念前，我们要先看明白JOS内存分布
<pre><code>
 * Virtual memory map:                                Permissions
 *                                                    kernel/user
 *
 *    4 Gig -------->  +------------------------------+
 *                     |                              | RW/--
 *                     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                     :              .               :
 *                     :              .               :
 *                     :              .               :
 *                     |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| RW/--
 *                     |                              | RW/--
 *                     |   Remapped Physical Memory   | RW/--
 *                     |                              | RW/--
 *    KERNBASE, ---->  +------------------------------+ 0xf0000000      --+
 *    KSTACKTOP        |     CPU0's Kernel Stack      | RW/--  KSTKSIZE   |
 *                     | - - - - - - - - - - - - - - -|                   |
 *                     |      Invalid Memory (*)      | --/--  KSTKGAP    |
 *                     +------------------------------+                   |
 *                     |     CPU1's Kernel Stack      | RW/--  KSTKSIZE   |
 *                     | - - - - - - - - - - - - - - -|                 PTSIZE
 *                     |      Invalid Memory (*)      | --/--  KSTKGAP    |
 *                     +------------------------------+                   |
 *                     :              .               :                   |
 *                     :              .               :                   |
 *    MMIOLIM ------>  +------------------------------+ 0xefc00000      --+
 *                     |       Memory-mapped I/O      | RW/--  PTSIZE
 * ULIM, MMIOBASE -->  +------------------------------+ 0xef800000
 *                     |  Cur. Page Table (User R-)   | R-/R-  PTSIZE
 *    UVPT      ---->  +------------------------------+ 0xef400000
 *                     |          RO PAGES            | R-/R-  PTSIZE
 *    UPAGES    ---->  +------------------------------+ 0xef000000
 *                     |           RO ENVS            | R-/R-  PTSIZE
 * UTOP,UENVS ------>  +------------------------------+ 0xeec00000
 * UXSTACKTOP -/       |     User Exception Stack     | RW/RW  PGSIZE
 *                     +------------------------------+ 0xeebff000
 *                     |       Empty Memory (*)       | --/--  PGSIZE
 *    USTACKTOP  --->  +------------------------------+ 0xeebfe000
 *                     |      Normal User Stack       | RW/RW  PGSIZE
 *                     +------------------------------+ 0xeebfd000
 *                     |                              |
 *                     |                              |
 *                     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                     .                              .
 *                     .                              .
 *                     .                              .
 *                     |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 *                     |     Program Data & Heap      |
 *    UTEXT -------->  +------------------------------+ 0x00800000
 *    PFTEMP ------->  |       Empty Memory (*)       |        PTSIZE
 *                     |                              |
 *    UTEMP -------->  +------------------------------+ 0x00400000      --+
 *                     |       Empty Memory (*)       |                   |
 *                     | - - - - - - - - - - - - - - -|                   |
 *                     |  User STAB Data (optional)   |                 PTSIZE
 *    USTABDATA ---->  +------------------------------+ 0x00200000        |
 *                     |       Empty Memory (*)       |                   |
 *    0 ------------>  +------------------------------+                 --+
 *
 * (*) Note: The kernel ensures that "Invalid Memory" is *never* mapped.
 *     "Empty Memory" is normally unmapped, but user programs may map pages
 *     there if desired.  JOS user programs map pages temporarily at UTEMP.
</code></pre>

######内核栈：kernel代码正常执行时，包括发生中断/异常陷入kernel时，kernel运行的栈称为“内核栈(kernel stack)”，它的esp寄存器指向当前CPU的栈顶，栈的线性地址范围为[kstacktop_i - KSTKSIZE, kstacktop_i -1]. （我们要知道在多核环境下，内核栈有多少个由核心数决定）中断/异常发生时，CPU自动切换到内核栈。之前我已经设置过内核栈。


######用户运行栈：在用户程序正常执行时，它所运行的栈称为“用户运行栈(normal user stack)”，它的esp寄存器指向USTACKTOP, 栈的线性地址范围是[USTACKTOP - PGSIZE, USTACKTOP - 1].

######用户异常栈：是用户自己定义相应的中断处理程序后，相应处理程序运行时的栈。注意这个处理程序是user mode的，而不是在内核中。它的esp寄存器指向UXSTACKTOP, 栈的线性地址范围是[UXSTACKTOP - PGSIZE, UXSTACKTOP - 1]，也是一页大小。

 

user mode发生异常，用户空间发生了一个page fault. 如果没有用户异常栈，理论上用户程序执行不下去了，它将陷入内核栈，然后在内核中处理page fault，然后返回用户程序。但是，这里我们将采用一个新的机制！也就是不再在内核中处理page fault，而是在user mode的用户异常栈上！

用户空间发生page fault后，首先陷入内核（用户运行栈->内核栈， user mode -> kernel mode），进入trap()处理中断分发，进入page_fault_handler().

然后内核中的page_fault_handler()在确认是用户程序而不是内核触发了page fault后（如果是内核发生page fault就直接panic了），在用户异常栈上压入一个UTrapframe作为记录保存现场的信息（注意这只是内存操作，还没有切换到用户异常栈）。

kernel将用户程序切换到用户异常栈（内核栈->用户异常栈, kernel mode -> user mode），然后就让用户程序重新运行了！

> 这里有两点要说明：1.现在用户程序是运行在用户异常栈（而不是用户运行栈）上，2.此时的eip被设置为page fault处理程序的起始地址curenv->env_pgfault_upcall。所以，接下来用户程序执行处理page fault的处理程序，它实际上是在用户空间中进行处理。

> page fault被处理完后，切换回用户运行栈（用户异常栈->用户运行栈, user mode -> user mode），用户程序重新运行。
弄清楚page fault发生前后“栈的切换”、“特权级别(mode)的切换”是非常重要的。

实际上判断用户程序是在kernel mode 还是 user mode主要是通过寄存器中 tf_cs == GD_KT来判断。

如果这个寄存器记录的值不是GD_KT那么，说明pagefault来自用户空间。

然后检查检查user mode pagefault handler即将调用异常处理函数的入口地址。即env_pgfault_upcall

这里涉及一个问题，判断当前的esp是否已经在用户异常栈中.

如果是的话，就要从esp再往下分配一个sizeof（UTrapframe）大小的空间，存放UTrapframe，否则从用户异常栈顶UXSTACKTOP分配UTrapframe。然后将寄存器里面的值eflags ，eip， esp，regs存入到UTrapframe结构体中。

######UTrapframe不同在于Trapframe的是env_tf.tf_eip env_tf.tf_esp

env_tf.tf_eip指向用户自己的异常处理函数，env_tf.tf_esp指向的是UTrapframe，保存完现场，然后系统把控制权交给用户定义的异常处理函数，然后系统进行调度。

理解了上述的过程，我对中断用户异常处理函数有了好的了解，下一步实现该代码即可。