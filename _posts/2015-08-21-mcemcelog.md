---
layout: post
title: "MCE与mcelog之前的交互"
categories: linux
tags: linux,mce,mcelog
---
MCE与mcelog之前的交互
=====================

mcelog是在用户空间实现记录解码MCA报告的硬件错误信息的工具，而MCA则是一个内核机制，用来收集硬件错误信息。但是这个时候仅仅是一系列的错误代码，需要依靠用户空间的mcelog进行解码。二者是如何协调的呢？

通过查看相关代码，二者交互的接口是/dev/mcelog ，而mcelog在这个字符设备上休眠，直到mcelog被唤醒，读取这个字符设备中的信息，谁来唤醒这个daemo呢？

我们看到在mce代码初始化的时候，初始化了一个工作队列和一个irq队列，二者本质上调用的内容是一样的。

<pre><code>
void mcheck_cpu_init(struct cpuinfo_x86 *c)
{
...
    if (__mcheck_cpu_ancient_init(c))
        return;
...
    machine_check_vector = do_machine_check;
 
...
    INIT_WORK(this_cpu_ptr(&mce_work), mce_process_work);
    init_irq_work(this_cpu_ptr(&mce_irq_work), &mce_irq_work_cb);
}
</code></pre>

在do_machine_check()最后的代码调用了mce_report_event()，而这个函数包括两个部分，一个是通知mcelog读取字符设备，一个是记录通知等待队列mce_work，调用mce_process_work()记录这个MCE错误，通常这个错误是SRAO等级。

<pre><code>
static void mce_report_event(struct pt_regs *regs)
{
         if (regs->flags & (X86_VM_MASK|X86_EFLAGS_IF)) {
                 mce_notify_irq();
 
                 mce_schedule_work();
                 return;
         }
         irq_work_queue(this_cpu_ptr(&mce_irq_work));
}
</code></pre>

irq_work_queue()也是通过irq队列唤醒mce_irq_work_cb()函数，这个函数实质上还是mce_notify_irq()与mce_schedule_work()。

<pre><code>
static void mce_irq_work_cb(struct irq_work *entry)
{
         mce_notify_irq();
         mce_schedule_work();
}
</code></pre>

所以mce与mcelog最最核心的两个函数就是mce_notify_irq()与mce_schedule_work()，我们看到mce_notify_irq()首先唤醒了mce_chrdev_wait，这个正是mce_chrdev_poll()所等待的事件，/dev/mcelog字符驱动poll函数。

<pre><code>
int mce_notify_irq(void)
{
...
         if (test_and_clear_bit(0, &mce_need_notify)) {
                 /* wake processes polling /dev/mcelog */
                 wake_up_interruptible(&mce_chrdev_wait);
 
                 if (mce_helper[0])
                         schedule_work(&mce_trigger_work);
...
}
static unsigned int mce_chrdev_poll(struct file *file, poll_table *wait)
{
         poll_wait(file, &mce_chrdev_wait, wait);
...
}
</code></pre>

然后又唤醒mce_trigger_work工作队列，这个工作队列唤醒了mce_do_trigger工作函数call_usermodehelper()，这个函数非常神奇的地方在于可以从内核空间直接调用用户空间进程！

<pre><code>
static void mce_do_trigger(struct work_struct *work)
{
         call_usermodehelper(mce_helper, mce_helper_argv, NULL, UMH_NO_WAIT);
}
</code></pre>

第二个核心函数就是mce_schedule_work()，通过工作队列mce_work最终还是mce_process_work()->memory_failure()。

<pre><code>
static void mce_schedule_work(void)
{
         if (!mce_ring_empty())
                 schedule_work(this_cpu_ptr(&mce_work));
}
</code></pre>

这里代码逻辑其实很简单，但是使用了两种内核机制，最终还是记录到ring_buffer，memory_failure()修复，唤醒mcelog解码硬件错误信息，并将其记录到/var/log/mcelog。

 

####具体查看 中断下半部的两种实现方式 中工作队列使用方式：

1. 通过下述宏动态创建一个工作:INIT_WORK(struct work_struct *work,void(*func)(void*),void *data);
2. 每个工作都有具体的工作队列处理函数，原型如下:void work_handler(void *data)
3. 将工作队列机制对应到具体的中断程序中，即那些被推后的工作将会在func所指向的那个工作队列处理函数中被执行。实现了工作队列处理函数后，就需要schedule_work函数对这个工作进行调度，就像这样：schedule_work(&work);
