---
layout: post
title: "setjmp和longjmp的另类使用"
categories: linux
tags: linux,syscall,thread
---
C语言的运行控制模型，是一个基于栈结构的指令执行序列，表现出来就是call/return: call调用一个函数，然后return从 一个函数返回。在这种运行控制模型中，每个函数调用都会对应着一个栈帧，其中保存了这个函数的参数、返回值地址、局部变量以及控制信息等内容。当调用一个 函数时，系统会创建一个对应的栈帧压入栈中，而从一个函数返回时，则系统会将该函数对应的栈帧从栈顶退出。正常的函数跳转就是这样从栈顶一个一个栈帧逐级地返回。

>    setjmp的返回值：直接调用该函数，则返回0；若由longjmp的调用，导致setjmp被调用，则返回val(longjmp的第二个参数)。

之前看APEU的相关章节，setjmp和longjmp只是一个跨函数跳转的库函数调用，可以作为后悔药使用，但是今天我发现这个库函数可以作为协程使用。协程我之前一直不理解，认为有了进程线程就可以了，没有必要存在协程，但是发现在不支持这些多线程多进程的操作系统平台上协程意义重大。

这个时候协程就可以派上用场了，我们可以依赖协程模拟多进程这种需求，我们需要写一个thread库供协程调用，具体的thread工作步骤就是：

1. 存储当前线程所在上下文，设置一个存储队列专门存储thread context
2. 为每个线程分配一个stack空间
3. 将esp指向栈顶，eip指向要执行代码的entry，当然包括参数arg，arg具体调用方式就是（current->entry(current->arg)），这一个非常相似于c++中的委托
4. 当需要调度线程时，将当前执行代码设置setjmp，保存线程结构体中的thread context到具体全局的数组
5. 如果需要调度另外一个线程，使用longjmp跳入到线程结构thread context

当然了在linux下有glibc提供相关库函数实现跳转，咱们不必再次造轮子，但是在裸机上，或者一种新的体系结构中，我们必须自行实现setjmp和longjmp，这其中不可避免的会使用到asm。比如setjmp，首先要将返回地址和frame pointer压入栈，考虑到栈自高地址向低地址方向生长，故esp-8，然后再压入其他通用寄存器。而longjmp恢复某个线程·的上下文环境，必须指定存储context位置 ，然后将返回地址复制给eax，然后执行跳转。

<pre><code>	
struct jmp_buf
{
       unsigned j_sp;  // 堆栈指针寄存器
       unsigned j_ss;  // 堆栈段
       unsigned j_flag;  // 标志寄存器
       unsigned j_cs;  // 代码段
       unsigned eip;  // 指令指针寄存器
       unsigned ebp; // 基址指针
       unsigned edi;  // 目的指针
       unsigned j_es; // 附加段
       unsigned j_si;  // 源变址
       unsigned j_ds; // 数据段
};
</code></pre>

具体线程切换伪代码：

<pre><code>	
void wthread_yield()
{
   ...
   if(current){
        if(setjmp(current->...)!=0)
             return;
        push(...)
   }
   current = next;
   longjmp(current->...)
}
</code></pre>

考虑到执行setjmp和longjmp必须是一个控制main线程，必须由控制线程控制调用线程切换，其他线程可以主动让出时间片。这时我们必须定义一个全局变量保存线程上下文，然后维护这个数组，至于具体的逻辑形式可以是队列可以是环形队列队列等。编写thread库务必保证线程安全，不能破坏线程返回地址，否则容易core dump。

另外在linux下，可以使用这两个系统调用实现C下的异常处理try/catch，至于在setjmp和longjmp之前存在的变量务必使用volatile声明。

 
###参考：

[http://stackoverflow.com/questions/2560792/multitasking-using-setjmp-longjmp#comment33335405_2560792](http://stackoverflow.com/questions/2560792/multitasking-using-setjmp-longjmp#comment33335405_2560792)

[http://www.cnblogs.com/lq0729/archive/2011/10/23/2222117.html](http://www.cnblogs.com/lq0729/archive/2011/10/23/2222117.html)

[http://www.cnblogs.com/lienhua34/archive/2012/04/22/2464859.html](http://www.cnblogs.com/lienhua34/archive/2012/04/22/2464859.html)