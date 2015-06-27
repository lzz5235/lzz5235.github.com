---
layout: post
title: "The handler of Kthread and Processes"
categories: linux
tags: signal,kthread,process
---
内核线程与用户进程在信号处理上的区别
===================================
在上一篇博客里面，我分析了信号在内核中处理的时机，发现对于内核线程没有类似于用户态程序信号处理的机制。后来我发邮件问了kthread的维护者Tetsuo Handa，他明确的给出了我内核线程没有类似于用户进程发送SIGSTOP将进程停止的机制。这个也就意味着我们要想让内核线程接收信号，并进行处理，必须在创建kernel thread代码中显式的允许某个信号。

进程对信号的响应

* 忽略信号：大部分信号可被忽略，除SIGSTOP和SIGKILL信号外（这是超级用户杀掉或停掉任意进程的手段）。

* 捕获信号：注册信号处理函数，它对产生的特定信号做处理。

* 让信号默认动作起作用：unix内核定义的默认动作，有5种情况：
a) 流产abort：终止进程并产生core文件。
b) 终止stop：终止进程但不生成core文件。
c) 忽略：忽略信号。
d) 挂起suspend：挂起进程。
e) 继续continue：若进程是挂起的，则resume进程，否则忽略此信号。

通常意义上来说内核线程对于信号是不处理的，如果想显式的让kernel thread支持信号，必须在内核线程中开启signal。编程框架类似于

<pre><code>
static int thread_process(void *arg)
{
....
    allow_signal(SIGURG);
    allow_signal(SIGTERM);
    allow_signal(SIGKILL);
    allow_signal(SIGSTOP);
    allow_signal(SIGCONT);  
...
    for ( ; !remove_mod; ) {
        /* Avoid infinite loop */
        msleep(1000);
        if (signal_pending(current)) {
                siginfo_t info;
                unsigned long signr;
                signr = dequeue_signal_lock(current, &current->blocked, &info);
                switch(signr) {
                        case SIGSTOP:
                                printk(KERN_DEBUG "thread_process(): SIGSTOP received.\n");
                                set_current_state(TASK_STOPPED);
                                schedule();
                                break;
                        case SIGCONT:
                                printk(KERN_DEBUG "thread_process(): SIGCONT received.\n");
                                set_current_state (TASK_INTERRUPTIBLE);
                                schedule();
                                break;
 
                        case SIGKILL:
                                printk(KERN_DEBUG "thread_process(): SIGKILL received.\n");
                                break;
                        //      goto die;
 
                        case SIGHUP:
                                printk(KERN_DEBUG "thread_process(): SIGHUP received.\n");
                                break;
                        default:
                                printk(KERN_DEBUG "thread_process(): signal %ld received\n", signr);
                        }
        }
        schedule_timeout_interruptible(msecs_to_jiffies(1));
    }
    return 0;
}
</code></pre>

在用户态下，我们只需要编写信号处理函数，然后使用signal(sig,handler)方式将信号处理函数与特定信号连接。向内核线程发信号与用户态进程发送信号都是发送某个特定特定pid号,比如19号信号是SIGSTOP，那么我们使用kill -19 pid即可。具体[pid解释](http://www.jb51.net/LINUXjishu/173601.html)。

创建内核线程，拥有两种方式1) kthread_create() 2) kernel_thread() 函数，虽然都是创建内核线程，但是二者在原理上不同。kthread_create() 创建的线程是挂在kthreadd()上面，kthread_create创建的内核线程有干净的上那上下文环境，适合于驱动模块或用户空间的程序创建内核线程使用，不会把某些内核信息暴露给用户程序。而kernel_thread()创建的线程来自于init进程。

所以我们推荐使用kthread_create()这种感觉方式创建内核线程，这种方式有利于模块的加载与卸载，有的时候kernel_thread创建的线程不容易卸载，只能通过reboot处理这种问题。

另外我们要非常注意内核线程的可重入性，在线程中使用函数必须保证函数是线程安全的，有些函数并不保证线程安全，如果我们在一个模块中修改全局变量，很有可能导致数据的不一致性，这里有必要要加锁。

 

###参考：

[http://www.spongeliu.com/165.html](http://www.spongeliu.com/165.html)

[http://blog.csdn.net/maimang1001/article/details/16906451](http://blog.csdn.net/maimang1001/article/details/16906451)