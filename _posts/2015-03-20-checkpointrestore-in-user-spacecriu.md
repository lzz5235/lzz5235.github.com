---
layout: post
title: "Checkpoint/Restore in user space:CRIU"
categories: linux
tags: C/R
---
CRIU 是一款目前流行的应用程序级的检查点恢复程序，这个基于OpenVZ 项目，但是OpenVZ项目最大的弊端是需要修改原有kernel。而CRIU则尽可能将程序主体放在用户空间，内核空间只保留必要的system call。

目前OpenVZ的开发，只停留在kernel 2.6.32上面，主要开发人员已经把他们的开发重点放在CRIU上面。

用户态下的CRIU程序我们不会细说，我们主要关注kernel中CRIU。包括两部分：1）需要一种mechanism去dump kernel关于该进程的某个特定信息。2）将状态信息传递给内核进行恢复。

CRIU的目标是允许整个application的运行状态可以被dump，这里就要去dump非常多的与这个application相关的信息，主要包括[1]:

* virtual memory map
* open files
* credential
* timer
* PID
* parent PID
* share resources

###dump 一个特定application的途径就是:
* Parasite code[2] 这个代码可以hack进一个特定进程，对进程透明的进行监控，获取文件描述符。dump memory content。实际原理就是在正常程序执行前，先执行Parasite code，实际的例子就是getitimer()和sigaction()。
* Ptrace 可以迅速freeze processes，注入parasite code。
* Netlink 获取 sockets，netns信息。
* 获取procfs 中特定PID的内容，/proc/PID/maps  /proc/PID/map_files/ /proc/PID/status  /proc/PID/mountinfo ，其中/proc/PID/map_files。这个map_files包括文件，网络等

Parasite code不是专门为CRIU设计，而是kernel的加入的特性，而CRIU使用了Parasite code去调用某些只能是application自己调用的system call，比如getitimer()。

除了一些特殊的system call，另外一些call可以由任意形式的程序进行调用，比如sched_getscheduler()获取调度器，使用sche_getparam()获取进程调度参数。

Ptrace 是一个system call，使用这个ptrace，可以做到控制目标进程，包括目标状态的内部信息，常用于debug和其他的代码分析工具。在kernel 3.4之前，ptrace非常依赖signal与目标进程交互，这就意味会打断进程执行，非常类似于gdb等工具，而加入PTRACE_SEIZE并不会停止进程。

ptrace新特性的引入，使得CRIU可以用来对于某个特定application进行checkpoint。

* Restore一个application：
* Collect shared object
* Restore namespace
* 创建进程树，包括SID，PGID，恢复继承
* files，socket，pipes
* Restore per-task properties
* Restore memory
* Call sigreturn

###特定kernel的feature:
* Parasite code[2]
* 如果一个程序打开了一系列的各种形式的文件，kernel在内核中会保存一个文件描述符表来记录该application打开哪些文件，在恢复时，CRIU要重新打开该这些文件，以相同的fd号。在恢复某些特定的pid 的application，发现pid被占用，如果我们想要恢复这个进程，而且继续使用这个pid值，CRIU在内核中加入一个API来控制下几个fork即将分配的pid值，主要是/proc/sys/kernel/ns_last_pid 。主要是向具体参见：http://lwn.net/Articles/525723/
* kernel还添加了kcmp()的system call，用来比较两个进程是否共享一个kernel资源。这个就用在父进程打开一系列的share resource，然后fork()。子进程继承父进程的resource，这时kcmp()派上用场。
* /proc/PID/map_files
* prctl拓展来设置匿名的，私有的对象。eg: task/mm object
* 通过netlink dump socket信息。在scoket恢复中，相比于/proc file，通过这个可以获取更多的socket信息，通过这些信息，CRIU使用getsockopt(),setsockopt()恢复socket链接。
* TCP repair mode
* virtual net device indexes，在一个命名空间中恢复网络设备
* socket peeking offset
* Task memory tracking，用于增量快照与线上迁移。
 
总的来说CRIU与OpenVZ有几分相似，二者最大的区别就是OpenVZ需要修改内核，非常不便，而CRIU依赖kernel加入的systemcall完成，对于内核没有要求，非常轻便。

而BLCR也是根据某个特定kernel 版本开发，它由两个kernel module，用户态lib工具组成。使用BLCR恢复进程，进程必须依赖libcr库，或者编译时将libcr加入。这个显然对于老旧代码非常不便。BLCR最新版本发布的时候2013.1

而CRIU 截止目前最新版本发布在2015.3.2 ，可以看出CRIU开发非常活跃。

 

###参考:
[1] [http://lwn.net/Articles/525675/](http://lwn.net/Articles/525675/)

[2] [http://lwn.net/Articles/454304/](http://lwn.net/Articles/454304/)

###BCLR:

[http://blog.csdn.net/myxmu/article/details/8948258](http://blog.csdn.net/myxmu/article/details/8948258)

[http://blog.csdn.net/myxmu/article/details/8948265](http://blog.csdn.net/myxmu/article/details/8948265)