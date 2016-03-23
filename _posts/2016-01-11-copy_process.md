---
layout: post
title: "内核函数copy_process()分析"
categories: linux
tags: kernel
---
内核通过调用函数copy_process()创建进程，copy_process()函数主要用来创建子进程的描述符以及与子进程相关数据结构。这个函数内部实现较为复杂，在短时间内，对于内部详细代码原理和实现并不能全部理解。因此，接下来的分析侧重于copy_process()的执行流程。

1. 定义返回值变量和新的进程描述符。

2.  对clone_flags所传递的标志组合进行合法性检查。当出现以下四情况时，返回出错代号：

* CLONE_NEWNS和CLONE_FS同时被设置。前者标志表示子进程需要自己的命名空间，而后者标志则代表子进程共享父进程的根目录和当前工作目录，两者不可兼容。
* CLONE_NEWUSER和CLONE_FS同时被设置。
* CLONE_THREAD被设置，但CLONE_SIGHAND未被设置。如果子进程和父进程属于同一个线程组（CLONE_THREAD被设置），那么子进程必须共享父进程的信号（CLONE_SIGHAND被设置）。
* CLONE_SIGHAND被设置，但CLONE_VM未被设置。如果子进程共享父进程的信号，那么必须同时共享父进程的内存描述符和所有的页表（CLONE_VM被设置）。

3. 调用security_task_create()和后面的security_task_alloc()执行所有附加的安全性检查。

4. 调用dup_task_struct()为子进程分配一个内核栈、thread_info结构和task_struct结构。

<pre><code>
p = dup_task_struct(current);
       if (!p)
              goto fork_out;
 </code></pre>
 
这个dup_task_struct函数首先定义创建了指向task_struct和thread_inof结构体的指针。然后让子进程描述符中的thread_info字段指向ti变量；最后让子进程thread_info结构中的task字段指向tsk变量。然后返回tsk，这个时候子进程和父进程的描述符中的内容是完全相同的。在后面的代码中，我们将会看到子进程逐渐与父进程区分开。

<pre><code>
static struct task_struct *dup_task_struct(struct task_struct *orig)
{
        struct task_struct *tsk;
        struct thread_info *ti;
        int node = tsk_fork_get_node(orig);
        int err;
  
        tsk = alloc_task_struct_node(node);
        if (!tsk)
                return NULL;
 
        ti = alloc_thread_info_node(tsk, node);
        if (!ti)
                goto free_tsk;
 
        err = arch_dup_task_struct(tsk, orig);
        if (err)
                goto free_ti;
        tsk->stack = ti;
 </code></pre>

5.开始设置子进程的task_struct

根据clone_flags的值继续更新子进程的某些属性。将 nr_threads++，表明新进程已经被加入到进程集合中。将total_forks++，以记录被创建进程数量。

这部分工作还包含初始化双链表、互斥锁和描述进程属性的字段等，其中包括大量的与cgroup相关的属性，。它在copy_process函数中占据了相当长的一段的代码，不过考虑到task_struct结构本身的复杂性，也就不奇怪了。

如果上述过程中某一步出现了错误，则通过goto语句跳到相应的错误代码处；如果成功执行完毕，则返回子进程的描述符p。do_fork()执行完毕后，虽然子进程处于可运行状态，但是它并没有立刻运行。至于子进程合适执行这完全取决于调度程序schedule()。

 [http://lxr.free-electrons.com/source/kernel/fork.c#L1242](http://lxr.free-electrons.com/source/kernel/fork.c#L1242)