---
layout: post
title: "Process Control踩过的坑"
categories: linux
tags: Process,Control
---
进程控制踩过的坑
================
1. fork()与vfork()非常相似，但是使用场景有一些不同，vfork()主要用来创建子进程，然后执行exec()一个新的程序，不会发生COW(fork()出来的子进程exec()会产生COW，所以vfork()更加快速)，vfork()可以保证子进程先运行，调用exec()、exit之后才会被调度，如果子进程依赖父进程产生一些动作的话，可能产生死锁。

2. vfork()在父进程空间中运行，这个导致子进程可以修改父进程的值！

3. 之前在C/S模型下Server 中fork()的健壮性中说过，fork()产生的子进程退出后，发送SIGCHLD信号，如果不及时使用wait方式处理的话，会产生僵尸进程。反过来，如果父进程先停止，那么子进程退出时，会向init进程发送SIGCHLD信号。

4. wait()与waitpid()都可以接受终止子进程发送的信号，wait()是waitpid()的简化版本，wait()返回任意一个终止子进程的状态，waitpid()可以接受特定子进程的信号。

5. 按照之前第3条所叙述的，我们可以利用这个init领养子进程规则让init管理孤儿进程，这里有一个技巧：fork()两次！

<pre><code>
int main(void)
{
        pid_t pid;
        if ((pid = fork()) less 0) {
             err_sys("fork error");
        } else if (pid == 0) { /* first child */
             if ((pid = fork()) less 0)
                  err_sys("fork error");
             else if (pid > 0)
                  exit(0); /* parent from second fork == first child */
//这个exit(0)退出的就是第一次fork()出来的子进程，也是第二次fork()的
//父进程，当这个进程退出后，也就意味着第二次fork()出来的子进程变成
//孤儿进程，直接由init接管！
/*
* We’re the second child; our parent becomes init as soon
* as our real parent calls exit() in the statement above.
* Here’s where we’d continue executing, knowing that when
* we’re done, init will reap our status.
*/
//下面这段是第二次fork()出来子进程执行的代码段
            sleep(2);//必须保证第二次fork()出来的父进程先退出！
            printf("second child, parent pid = %ld\n", (long)getppid());
            exit(0);
        }
        if (waitpid(pid, NULL, 0) != pid) /* wait for first child */
            err_sys("waitpid error");
/*
* We’re the parent (the original process); we continue executing,
* knowing that we’re not the parent of the second child.
*/
        exit(0);
}
</code></pre>

这个代码设计的很精巧，开始我没有看懂，仔细分析才可以。

6.对于某些父子进程拥有竞争条件的代码，必须要使用信号机制或者管道机制实现父子进程同步,其中TELL_WAIT(),TELL_PARENT(),WAIT_PARENT(),TELL_CHILD(pid),WAIT_CHILD()可以使用不同的机制定义，从而实现父子进程的有序执行！

<pre><code>
TELL_WAIT(); /* set things up for TELL_xxx & WAIT_xxx */
 if ((pid = fork()) less 0) {
  err_sys("fork error");
 } else if (pid == 0) { /* child */
 /* child does whatever is necessary ... */
 TELL_PARENT(getppid()); /* tell parent we’re done */
 WAIT_PARENT(); /* and wait for parent */
 /* and the child continues on its way ... */
 exit(0);
 }
/* parent does whatever is necessary ... */
TELL_CHILD(pid); /* tell child we’re done */
WAIT_CHILD(); /* and wait for child */
/* and the parent continues on its way ... */
exit(0);
</code></pre>

7.使用信号机制来实现父子进程同步的话，可以自定义SIGUSR1，SIGUSR2的方式，在main()开始部位，设置中断处理函数，函数修改一个全局volatile sig_atomic类型的变量sigflag，然后在等待函数中，轮训挂起等待信号，直至进程处理信号，跳出这个循环：

<pre><code>
while (sigflag == 0)
       sigsuspend(&zeromask); /* and wait for parent */
sigflag = 0;
</code></pre>

8.使用pipe，可以在等待函数中读管道，在通知函数中写管道，达到父子进程的同步！

<pre><code>
void TELL_PARENT(pid_t pid)
{
    if (write(pfd2[1], "c", 1) != 1)
        err_sys("write error");
}
void WAIT_PARENT(void)
{
    char c;
    if (read(pfd1[0], &c, 1) != 1)
        err_sys("read error");
    if (c != ’p’)
        err_quit("WAIT_PARENT: incorrect data");
}
</code></pre>

 

参考：
APUE P185，P270，P402