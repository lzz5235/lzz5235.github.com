---
layout: post
title: "内核开发特点&进程管理（读书笔记）"
categories: [read]

tags: [Kernel,Process,Read]
 


---
内核开发特点&进程管理（读书笔记）
================================
在看内核编程前，几点重要的建议：

1.内核编程是不能访问C库。使用printk（） 而不是printf（）头文件使用<linux/string.h>

2.内核编程必须使用GNU C。使用static inline 限定，C语言中嵌入汇编，要知道对应的体系结构。

3.内核编程缺乏用户空间那样的内存保护机制。用户态有SIGSEGV，内核错误导致oops，内核的内存不会分页，用掉就少一个字节。

4.内核编程float很难使用。

5.内核只有一个很小的定长堆栈。内核栈在编译时配置，32位机8KB   64位机16KB。

6.由于内核支持异步中断，抢占和SMP，因此必须时刻注意同步和并发。要时刻注意对共享资源进行保护，我们要采用spinlock与mutex

7.要考虑可移植性的重要性。字节对齐，64位对齐。

#####pid最大值在/proc/sys/kernel/pid_max进行指定提高上限。

其中task_struct结构体声明在/usr/src/kernels/linux-3.11.8/include/linux/sched.h line 1027到1407之间，大概400行左右 

因为linux kernel实际上是一个宏，这个结构体里超级多的ifdef与endif，显然这些在编译内核的时候用得到。当然也有ftrace kprobe的定义！

fork()定义在kernel/fork.c中的do_fork()  ->copy_process（）拷贝父进程的代码段，数据段。

调用的大概路径就是dup_task_struct()为新进程创建内核栈等结构->复制父进程，并检查这个新进程是否合法，是否超过分配资源限制->copy_flags()表明是否被root权限调用  PF_SUPERPRIV位 并且将struct中各种位都要设成初始值 ->做扫尾工作，返回p。

#####vfork()与fork（）定义相同，区别在于拷贝共享数据段。vfork（）出来的子进程必须在使用exec（）或者_exit（）后，父进程才可以执行。非常适合多线程对共享数据段的操作。

<pre><code>
#include<sys/types.h>  
#include<unistd.h>  
#include<stdio.h>  
   
int main()  
{  
    pid_t pid;  
    int cnt = 0;  
    pid = vfork();  
    if(pid<0)  
        printf("error in fork!\n");  
    else if(pid == 0)  
    {  
        cnt++;  
        printf("cnt=%d\n",cnt);  
        printf("I am the child process,ID is %d\n",getpid());  
       _exit(0);  
    }  
    else 
    {  
        cnt++;  
        printf("cnt=%d\n",cnt);  
        printf("I am the parent process,ID is %d\n",getpid());  
    }  
    return 0;  
   
}  
</code>
</pre>

事实上普通fork（）实现的系统调用clone（）中的clone（SIGCHLD,0）,而vfork（）调用的是clone（CLONE_VFORK|CLONE_VM|SIGCHLD,0）
SIGCHLD就是复制父子进程，CLONE_VFORK父进程睡眠，子进程唤醒，CLONE_VM父子进程共享地址空间，详细的定义在

 
exit（）这个函数主要是由do_exit()实现。kernel/exit.c实现。
我们先来看include/linux/sched.h中的struct task_struct结构.

<pre><code>
struct task_struct {
    volatile long state;    /* -1 unrunnable, 0 runnable, >0 stopped */
    void *stack;
    atomic_t usage;
    unsigned int flags; /* per process flags, defined below */
    unsigned int ptrace;
 
#ifdef CONFIG_SMP
    struct llist_node wake_entry;
    int on_cpu;
#endif
    int on_rq;
..............
</code></pre>

task_struct中的第一项就是status 0是运行，>0是各种停止状态。比如exiting就是status赋值TASK_DEAD 64
然后放弃进程占用mm_struct（mm_struct结构描述了一个进程的整个虚拟地址空间。）exit_sem()放弃等待IPC信号。
然后调用_exit_files(),_exit_fs()。
exit_notify()向父进程发送信号。进程状态设置为TASK_ZOMBIE,然后调用schedule（）切换进程。然后此时进程存在的意义就是等待父进程检索。
这种等待父进程检索的进程，只有父进程获悉子进程终结的相关信息后，才可以真正释放掉。如果父进程先于子进程退出，先在线程组里找，找不到的话就是init进程。然后找到父进程，就可以顺利清除僵尸进程了。


