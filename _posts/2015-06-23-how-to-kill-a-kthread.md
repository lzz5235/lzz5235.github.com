---
layout: post
title: "How to kill a kthread?"
categories: linux
tags: kthread
---
如何杀死一个内核线程
============
首先明确杀死一个进程与杀死一个kthread是不同的，杀死进程的时机是进程从内核态返回到用户态检查_TIF_SIGPENDING标志位，进一步进入到处理信号的函数进行处理杀死这个进程。

内核线程运行在整个内核之上，，如果不返回，则不可能检查信号，所以内核的线程实质上的停止与启动必须由线程本身状态决定，不允许随意杀死。如果这个线程正在持有某个全局锁时，强制杀死kthread会造成整个内核的死锁。所以目前kernel对于内核线程的停止主要依赖于线程内部的停止。

###一种方式
发送信号，对于内核线程默认是对于信号是忽略的，所以我们要想停止一个线程必须在线程内部使用allow_signal(SIGKILL)方式，然后在内核线程代码的某个部位处理这个信号。所以发送信号的时机非常重要，如果当前kthread正在进行某些业务逻辑，那么发送SIGKILL无效。

###另外一种方式
使用目前kernel提供工具函数int kthread_stop(struct task_struct *k) 用来对某个kthread进行停止。这个函数仅仅限于kthread_create()创建的内核线程，通过这个函数创建的内核线程都会被挂在kthreadd 内核线程树上。这种方式也可以被看作是一种发送信号的方式，但是这些函数已经被提供出来供编写者用来停止内核线程。线程内部必须显式的检查THREAD_SHOULD_STOP信号，从而使得线程return或者使用do_exit()退出线程[1]。否则无法停止内核线程。

当kthread_create()创建的内核线程时：

<pre><code>
kthread_create
  -> kthread_create_on_node                              // in kthead.c
      -> adds your thread request to kthread_create_list
          -> wakes up the kthreadd_task
</code></pre>
当唤醒kthreadd_task时，这个函数会运行kthreadd()。

<pre><code>
pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
...
kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
</code></pre>

kthreadd()这个函数会调用kthread()函数。kthread()函数 调用用户定义的内核线程函数。

<pre><code>
kthreadd                                                 // all in kthread.c
  -> create_kthread
      -> kernel_thread(kthread, your_kthread_create_info, ...)
</code></pre>

kthread()函数会调用我们自己创建的内核线程函数，当需要停止的时候，检查KTHREAD_SHOULD_STOP位，当返回后会将ret值传递到do_exit(ret)，这个也就是我们不用显示调用do_exit()的原因。

<pre><code>
kthread
  -> initialization stuff
    -> schedule() // allows you to cancel the thread before it's actually started
      -> if (!should_stop)
          -> ret = your_thread_function()
            -> do_exit(ret)
</code></pre>

注意：内核线程return时，默认调用do_exit(ret)，如果直接使用do_exit()退出线程，那么必须保证task_struct不被释放否则当继续执行kthread_stop()会释放一个无效的task_struct，导致发生Oops。[4]

当需要停止目标内核线程，kernel会获取当前描述目标内核线程状态的结构体kthread，设置KTHREAD_SHOULD_STOP标示位，然后唤醒这个目标线程，当前进程调用wake_for_completion(&kthread->exited)睡眠，被唤醒的条件其实就是这个目标内核线程的task_struct 上的vfork_done完成，这个标志位在do_exit()中被设置。当前进程/内核线程等待目标内核线程结束的过程时不可中断的，直到目标内核线程退出，最后释放task_struct结构体，这样就可以安全的停止当前线程。

<pre><code>
int kthread_stop(struct task_struct *k)
{
        struct kthread *kthread;
        int ret;
 
        trace_sched_kthread_stop(k);
 
        get_task_struct(k);
        kthread = to_live_kthread(k);
        if (kthread) {
            set_bit(KTHREAD_SHOULD_STOP, &kthread->flags);
            __kthread_unpark(k, kthread);
            wake_up_process(k);
            wait_for_completion(&kthread->exited);
        }
        ret = k->exit_code;
        put_task_struct(k);
 
        trace_sched_kthread_stop_ret(ret);
        return ret;
}
</code></pre>

上面的代码必须确保task_struct有效，如果无效，调用这个函数会发生Oops。

在内核线程中的业务处理逻辑外使用kthread_should_stop()检查当前线程的KTHREAD_SHOULD_STOP标志位，如果被设置，退出循环，就要执行线程的退出操作。

<pre><code>
do {
        //do business
} while(!kthread_should_stop());
</code></pre>

[1] [http://v4l.videotechnology.com/dwg/kernelthreads/kernelthreads.html](http://v4l.videotechnology.com/dwg/kernelthreads/kernelthreads.html)

[2] [http://lwn.net/Articles/65178/](http://lwn.net/Articles/65178/)

[3] [http://blog.csdn.net/chinayangbo2011/article/details/8923731](http://blog.csdn.net/chinayangbo2011/article/details/8923731)

[4] [http://stackoverflow.com/questions/10177641/proper-way-of-handling-threads-in-kernel](http://stackoverflow.com/questions/10177641/proper-way-of-handling-threads-in-kernel)
