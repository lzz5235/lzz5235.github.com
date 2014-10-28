---
layout: post
title: "Two way to implement interruption"
categories: kernel
tags: interruption
---
中断下半部的两种实现方式
=========================
这篇文章的前提是我们已经知道了中断上下半部的划分。

中断的下半部有两种实现方式：tasklet与工作队列，软中断。不管是那种机制，它们均为下半部提供了一种执行机制，比上半部灵活多了。至于何时执行，则由内核负责。

#1.tasklet的实现方式与特点
tasklet由tasklet_struct结构体来表示，每一个这样的结构体就表示一个tasklet。

在<linux/interrupt.h>中

<pre><code>
tasklet_struct
{
    struct tasklet_struct *next;
    unsigned long state;
    atomic_t count;
    void (*func)(unsigned long);
    unsigned long data;
};
</code></pre>

第一个成员代表链表中的下一个tasklet。第二个成员代表此刻tasklet的状态，一般为TASKLET_STATE_SCHED,表示此tasklet已被调度且正准备运行；此成员变量还可取TASKLET_STATE_RUN，表示正在运行，但只用在多处理器的情况下。

count成员是一个引用计数器，只有当其值为0时候，tasklet才会被激活；否则被禁止，不能被执行。而接下来的func变量很明显是一个函数指针，它指向tasklet处理函数，这个处理函数的唯一参数为data。

使用tasklet前，首先创建一个tasklet_struct类型的变量。通常有两种方法：静态创建和动态创建。
在<linux/interrupt.h>中的两个宏：

<pre><code>
464#define DECLARE_TASKLET(name, func, data) \
465struct tasklet_struct name = { NULL, 0, ATOMIC_INIT(0), func, data }
466
467#define DECLARE_TASKLET_DISABLED(name, func, data) \
468struct tasklet_struct name = { NULL, 0, ATOMIC_INIT(1), func, data }
</code></pre>

就是我们进行静态创建tasklet的两种方法。

* 通过第一个宏创建的tasklet处于激活状态，再通过调度函数被挂起尽而被内核执行；

* 而通过第二个宏创建的tasklet处于禁止状态。从两个宏的定义可以看到，所谓的静态创建就是直接定义个一个名为name的tasklet_struct类型的变量，并将宏中各个参数相应的赋值给这个name变量的各个成员。注意，两个宏在功能上差异就在于对name变量count成员的赋值上，具体原因在第一部分已经说明。

<pre><code>
//在arch/x86/include/asm/atomic.h中
15#define ATOMIC_INIT(i)  { (i) }
//在linux/types.h中
190typedef struct {
191        int counter;
192} atomic_t;
</code></pre>

与静态创建相对的是动态创建，通过给tasklet_init函数传递一个事先定义的指针，来动态创建一个tasklet。这个函数源码如下。

<pre><code>
470void tasklet_init(struct tasklet_struct *t,
471                  void (*func)(unsigned long), unsigned long data)
472{
473        t->next = NULL;
474        t->state = 0;
475        atomic_set(&t->count, 0);
476        t->func = func;
477        t->data = data;
478}
</code></pre>

不过这里还是要特别说明一下atomic_set函数：

<pre><code>
//在arch/x86/include/asm/atomic.h中
35static inline void atomic_set(atomic_t *v, int i)
36{
37        v->counter = i;
38}
</code></pre>

首先tasklet_init当中，将&t->count传递给了此函数。也就是说将atomic_t类型的成员count的地址传递给了atomic_set函数。而我们在此函数中却要为count变量中的成员counter赋值。如果说我们当前要使用i，那么应该是如下的引用方法：t->count.i。

ok，通过上述两种方法就可以创建一个tasklet了。同时，你应该注意到不管是上述那种创建方式都有func参数。透过上述分析的源码，我们可以看到func参数是一个函数指针，它指向的是这样的一个函数：

<pre><code>
void tasklet_handler(unsigned long data);
</code></pre>

如同上半部分的中断处理程序一样，这个函数需要我们自己来实现。

创建好之后，我们还要通过如下的方法对tasklet进行调度：

<pre><code>
tasklet_schedule(&my_tasklet)
</code></pre>

通过此函数的调用，我们的tasklet就会被挂起，等待机会被执行。

下面我使用tasklet 方式写一个kernel module.我们使用触发键盘中断的方式自定义一个handler

<pre><code>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
 
static int irq;
static char* devname;
static struct tasklet_struct mytasklet;
 
module_param(irq,int,0644);
module_param(devname,charp,0644);
 
struct myirq
{
    int devid;
};
struct myirq mydev={1119};
 
static void mytasklet_handler(unsigned long data)
{
    printk("tasklet is working..\n");
}
 
// static irqreturn_t myirq_handler(int irq,void *dev)
// {
//  struct myirq mydev;
//  static int count=0;
//  mydev=*(struct myirq*)dev;
//  printk("key:%d..\n",count+1);
//  printk("devid:%d ISR is working..\n",mydev.devid);
//  printk("Bottom half will be working...\n");
//  tasklet_init(&mytasklet,mytasklet_handler,0);
//  tasklet_schedule(&mytasklet);
//  printk("ISR is leaving...\n");
//  count++;
//  return IRQ_HANDLED;
// }
 
static irqreturn_t myirq_handler(int irq,void* dev)
{
    static int count=0;
    if(count<10)
    {
        printk("-----------%d start--------------------------\n",count+1);
                printk("The interrupt handeler is working..\n");
                printk("The most of interrupt work will be done by following tasklet..\n");
                tasklet_init(&mytasklet,mytasklet_handler,0);
            tasklet_schedule(&mytasklet);
                printk("The top half has been done and bottom half will be processed..\n");
    }
    count++;
        return IRQ_HANDLED;
}
static int __init myirq_init()
{
    printk("Module is working....\n");
    if(request_irq(irq,myirq_handler,IRQF_SHARED,devname,&mydev)!=0)
    {
        printk("%s request IRQ:%d failed..\n",devname,irq);
        return -1;
    }
    printk("%s request IRQ:%d success..\n",devname,irq);
    return 0;
}
 
static void __exit myirq_exit()
{
    printk("Module is leaving...\n");
    free_irq(irq,&mydev);
    printk("%s request IRQ:%d success....\n",devname,irq);
}
 
module_init(myirq_init);
module_exit(myirq_exit);
MODULE_LICENSE("GPL");
</code></pre>

##module的编写主要是要有 module_init(myirq_init); 与  module_exit(myirq_exit);加入参数就是

module_param(irq,int,0644); module_param(devname,charp,0644);

使用方法：

* cat /proc/interrupts查看中断号，以确定一个即将要共享的中断号。本程序因为是与usb键盘（PS/2键盘 IRQ=1）共享16号中断线，因此irq=16；

* 使用如下命令就可以插入内核：

sudo insmod filename.ko irq=16 devname=myirq

* 再次查看/proc/interrupts文件，可以发现16号中断线对应的的设备名处多了myirq设备名；

* dmesg查看内核日志文件，可看到在中断处理程序中所显示的信息；

* 卸载内核模块；


<pre><code>
[75609.704894] -----------1 start--------------------------
[75609.704895] The interrupt handeler is working..
[75609.704897] The most of interrupt work will be done by following tasklet..
[75609.704902] The top half has been done and bottom half will be processed..
[75609.704904] myirq request IRQ:1 success....
[75609.705040] tasklet is working..
[76096.900571] Module is working....
[76096.900587] myirq request IRQ:16 success..
[76096.975295] -----------1 start--------------------------
[76096.975301] The interrupt handeler is working..
[76096.975303] The most of interrupt work will be done by following tasklet..
[76096.975304] The top half has been done and bottom half will be processed..
[76096.975314] tasklet is working..
[76097.847088] -----------2 start--------------------------
............
</code></pre>

简单的interruption模块就是这个样子。内核模块复杂的地方在与要弄懂内核复杂的数据结构。

 

#2.工作队列的实现方式与特点

工作队列（work queue）是另外一种将中断的部分工作推后的一种方式，它可以实现一些tasklet不能实现的工作，比如工作队列机制可以睡眠。这种差异的本质原因是，在工作队列机制中，将推后的工作交给一个称之为工作者线程（worker thread）的内核线程去完成（单核下一般会交给默认的线程events/0）。因此，在该机制中，当内核在执行中断的剩余工作时就处在进程上下文（process context）中。也就是说由工作队列所执行的中断代码会表现出进程的一些特性，最典型的就是可以重新调度甚至睡眠。

对于tasklet机制（中断处理程序也是如此），内核在执行时处于中断上下文（interrupt context）中。而中断上下文与进程毫无瓜葛，所以在中断上下文中就不能睡眠。

###因此，选择tasklet还是工作队列来完成下半部分应该不难选择。当推后的那部分中断程序需要睡眠时，工作队列毫无疑问是你的最佳选择；否则，还是用tasklet吧。

中断上下文与进程上下文有不同的地方。

* 进程上下文，就是一个进程在执行的时候，CPU的所有寄存器中的值、进程的状态以及堆栈中的内容。当内核需要切换到另一个进程时（上下文切换），它需要保存当前进程的所有状态，即保存当前进程的进程上下文，以便再次执行该进程时，能够恢复切换时的状态继续执行。上述所说的工作队列所要做的工作都交给工作者线程来处理，因此它可以表现出进程的一些特性，比如说可以睡眠等。

* 对于中断而言，是硬件通过触发信号，导致内核调用中断处理程序，进入内核空间。这个过程中，硬件的一些变量和参数也要传递给内核，内核通过这些参数进行中断处理，中断上下文就可以理解为硬件传递过来的这些参数和内核需要保存的一些环境，主要是被中断的进程的环境。因此处于中断上下文的tasklet不会有睡眠这样的特性。

##工作队列的使用
内核中通过下述结构体来表示一个具体的工作：

<pre><code>
struct work_struct
{
    unsigned long pending;//这个工作是否正在等待处理
    struct list_head entry;//链接所有工作的链表，形成工作队列
    void (*func)(void *);//处理函数
    void *data;//传递给处理函数的参数
    void *wq_data;//内部使用数据
    struct timer_list timer;//延迟的工作队列所用到的定时器
};
</code></pre>

而这些工作（结构体）链接成的链表就是所谓的工作队列。工作者线程会在被唤醒时执行链表上的所有工作，当一个工作被执行完毕后，相应的work_struct结构体也会被删除。当这个工作链表上没有工作时，工作线程就会休眠。

通过如下宏可以创建一个要推后的完成的工作:

<pre><code>
DECLARE_WORK(name,void(*func)(void*),void *data);
</code></pre>
也可以通过下述宏动态创建一个工作:

<pre><code>
INIT_WORK(struct work_struct *work,void(*func)(void*),void *data);
</code></pre>
与tasklet类似，每个工作都有具体的工作队列处理函数，原型如下:

<pre><code>
void work_handler(void *data)
</code></pre>

将工作队列机制对应到具体的中断程序中，即那些被推后的工作将会在func所指向的那个工作队列处理函数中被执行。

实现了工作队列处理函数后，就需要schedule_work函数对这个工作进行调度，就像这样：

<pre><code>
schedule_work(&work);
</code></pre>

这样work会马上就被调度，一旦工作线程被唤醒，这个工作就会被执行（因为其所在工作队列会被执行）。
所以在使用tasklet与工作队列的方式很相近。

在内核加载函数中，我们除了显示一些信息外，最重要的工作就是申请一根中断请求线，也就是注册中断处理程序。很明显，这一动作是通过request_irq函数来完成的。这个函数的原型如下：

<pre><code>
static int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,const char *name, void *dev);
</code></pre>

第一个参数是中断号，这个中断号对应的就是中断控制器上IRQ线的编号。

第二个参数是一个irq_handler_t类型个函数指针：

<pre><code>
typedef irqreturn_t (*irq_handler_t)(int, void *);
</code></pre>

handler所指向的函数即为中断处理程序，需要具体来实现。

第三个参数为标志位，可以取IRQF_DISABLED、IRQF_SHARED和IRQF_SAMPLE_RANDOM之一。在本实例程序中取IRQF_SHARED，该标志表示多个设备共享一条IRQ线，因此相应的每个设备都需要各自的中断服务例程。一般某个中断线上的中断服务程序在执行时会屏蔽请求该线的其他中断，如果取IRQF_DISABLED标志，则在执行该中断服务程序时会屏蔽所有其他的中断。取IRQF_SAMPLE_RANDOM则表示设备可以被看做是事件随见的发生源。

第四个参数是请求中断的设备的名称。可以在/proc/interface中查看到具体设备的名称，与此同时也可以查看到这个设备对应的中断号以及请求次数，甚至中断控制器的名称。

第五个参数为一个指针型变量。注意此参数为void型，也就是说通过强制转换可以转换为任意类型。这个变量在IRQF_SHARED标志时使用，目的是为即将要释放中断处理程序提供唯一标志。因为多个设备共享一条中断线，因此要释放某个中断处理程序时，必须通过此标志来唯一指定这个中断处理程序。习惯上，会给这个参数传递一个与设备驱动程序对应的设备结构体指针。关于中断程序，可参考这里的文章。

以上就是request_irq函数各个参数的意义。

####与中断处理程序的注册相对应的是free_irq函数，它会注销相应的中断处理程序，并释放中断线。这个函数一般被在内核模块卸载函数中被调用。
如果该中断线不是共享的，那么该函数在释放中断处理程序的同时也将禁用此条中断线。如果是共享中断线，只是释放与mydev对应的中断处理程序。除非该中断处理程序恰好为该中断线上的最后一员，此条中断线才会被禁用。


