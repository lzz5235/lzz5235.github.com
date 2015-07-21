---
layout: post
title: "Signal Handler所踩过的坑"
categories: linux
tags: signal,handler
---
信号处理函数所踩过的坑
======================
最近在看APUE的信号章节,在这里我总结下进程信号处理中应该注意的一些坑。Unix中有很多的信号是可以被进程接管，然后跳到信号处理函数中。

1. 有两个信号是无法被接管或者被忽略的SIGKILL与SIGSTOP

2. SIGHUP 是要出现在远程ssh一台主机时，连接意外断开时，系统会向所有与这个终端相关的控制进程发送SIGHUP。

3. 在liunx中SIGIO与SIGPOLL相同，默认是终止这个进程。

4. SIGTERM可以由进程编写者定义，当收到这个信号那么，进程可以自行做退出操作的扫尾工作，然后退出程序。

5. signal与sigaction功能相似，但是signal在不同平台上实现不同，应该使用sigaction进程信号的接管。

6. 交互式进程后台运行时，shell会将后台进程设置为对于中断和退出信号的处理方式设置为忽略SIG_IGN。也就是说当向进程发送SIGINT时，捕捉这种类型的代码:

<pre><code>
void sig_int(int), sig_quit(int);
if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    signal(SIGINT, sig_int);
if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
    signal(SIGQUIT, sig_quit);
</code></pre>

7.当父进程fork()一个子进程，子进程将会继承父进程的信号处理函数，这种方式在早期fork()一个子进程后会把这个子进程信号处理函数复位到默认值，我们不必在代码中这么做：

<pre><code>
int sig_int(); /* my signal handling function */
...
signal(SIGINT, sig_int); /* establish handler */
...
sig_int()
{
    signal(SIGINT, sig_int); /* reestablish handler for next time */
... /* process the signal ... */
}
</code></pre>

8.信号会发生在任何时刻，我们不能设置flag来使得进程进行忙等。下面这种代码在大多数情况下是正确的，但是如果信号发生在while()与pause()之间，会直接导致进程陷入睡眠，无法醒来。

<pre><code>
int sig_int(); /* my signal handling function */
int sig_int_flag; /* set nonzero when signal occurs */
main()
{
     signal(SIGINT, sig_int); /* establish handler */
...
     while (sig_int_flag == 0)
            pause(); /* go to sleep, waiting for signal */
...
}
sig_int()
{
    signal(SIGINT, sig_int); /* reestablish handler for next time */
    sig_int_flag = 1; /* set flag for main loop to examine */
}
</code></pre>

9.被中断的syscall（通常是慢速系统调用：read，write，open()(如果open不返回，就意味着进程会被永久的阻塞) etc.）必须显式的处理出错返回,在linux中被中断的syscall，会重启这个syscall，但是在当次的调用中，会将errno设置为EINTR,所以我们要对这个EINTR进行处理。如下面的代码：

<pre><code>
again:
if (( n = read(fd, buf, BUFFSIZE ) )  小于 0  )  
{
    if (errno == EINTR)
        goto again; /* just an interrupted system call */
    /* handle other errors */
}
</code></pre>

10.信号处理函数的可重入性。如果在信号处理函数中调用，会对进程主体的程序执行流造成破坏，产生Sigment fault。在内核中的实现，我发现为了实现进程处理函数在用户态执行，会将内核态的堆栈数据复制到用户空间的堆栈保存，返回用户空间，执行完sys_sigreturn() 再次陷入到内核，将正常程序的用户态堆栈硬件上下文拷贝到内核堆栈，并将之前备份在用户空间的堆栈还原到内核空间，完成这次中断处理函数。

> 不可重入性:(a) they are known to use static data structures, (b) they call malloc or free, or (c) they are part of the standard I/O library. Most implementations of the standard I/O library use global data structures in a nonreentrant way.

所以按照定义，为了保证函数是可重入的，需要做到一下几点：

* 不在函数内部使用静态或者全局数据
* 不返回静态或者全局数据，所有的数据都由函数调用者提供
* 使用本地数据，或者通过制作全局数据的本地拷贝来保护全局数据
* 如果必须访问全局数据，使用互斥锁来保护
* 不调用不可重入函数

getpwnam()函数是非可重入函数，他在中断处理函数中使用的话，就会修改原来应用程序的数据，导致程序出错

<pre><code>
#include "apue.h"
#include < pwd.h >
static void
my_alarm(int signo)
{
       struct passwd *rootptr;
       printf("in signal handler\n");
       if ((rootptr = getpwnam("root")) == NULL)
           err_sys("getpwnam(root) error");
        alarm(1);
}
int main(void)
{
       struct passwd *ptr;
       signal(SIGALRM, my_alarm);
       alarm(1);
       for ( ; ; ) {
           if ((ptr = getpwnam("sar")) == NULL)
               err_sys("getpwnam error");
           if (strcmp(ptr->pw_name, "sar") != 0)
               printf("return value corrupted!, pw_name = %s\n",ptr->pw_name);
       }
}
</code></pre>

这段代码中的rootptr其实最后都是指向ptr，这就是造成不可重入的关键！我们使用getpwnam_r()函数便可以正常工作。

<pre><code>
void sig_handler(int signo)
{
   struct passwd root_ptr;
   struct passwd *result;
   int s;
   char *buf;
   size_t bufsize;
 
   bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
   if(bufsize==-1)
      bufsize = 16384;
 
   buf = malloc(bufsize);
   if(buf==NULL){
      perror("malloc");
      exit(EXIT_FAILURE);
   }
 
   printf("in sig_handler\n");
   s = getpwnam_r("root",&root_ptr,buf,bufsize,&result);
   if(result == NULL){
      if(s==0)
          printf("Not found\n");
      else{
          // errno = s;
          perror("getpwnam_r");
      }
      exit(EXIT_FAILURE);
   }
   printf("pw_name = %s\n", root_ptr.pw_name);
   alarm(1);
}
</code></pre>

11.SIGCHLD这个信号非常特殊，这个信号很多时候与系统的信号实现相关。在linux平台上 SIGCHLD与SIGCLD等同，这里查看C/S模型下Server 中fork()的健壮性文章，我们需要在父进程信号处理函数中调用pid = wait(&stat);实现对于子进程退出的等待。

<pre><code>
void sig_zchild(int signo)
{
      pid_t pid;
      int stat;
 
      while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
           printf("child %d terminated\n", pid);
      return;
}
</code></pre>

12. kill() 函数负责将信号发送给进程或者进程组，raise()是进程向自己发送信号。一个程序全局只能有一个alarm()函数，如果多次调用，那么alarm()登记的值被新值代替。pause()使得调用进程挂起直至捕捉到一个信号，只有执行了一个信号处理函数返回后，pause()才返回。

<pre><code>
#include < signal.h >
#include < unistd.h >
static void
sig_alrm(int signo)
{
/* nothing to do, just return to wake up the pause */
}
unsigned int
sleep1(unsigned int seconds)
{
      if (signal(SIGALRM, sig_alrm) == SIG_ERR)
               return(seconds);
      alarm(seconds); /* start the timer */
      pause(); /* next caught signal wakes us up */
      return(alarm(0)); /* turn off timer, return unslept time */
}
</code></pre>

这个函数看似正确，但是有一个竞争条件，如果alarm（）后调用被阻塞，然后超时，pause()没有捕捉到信号，那么调用到pause()将永久挂起，这里我们要使用到longjmp() 与 setjmp() 可以使得信号处理函数返回到主函数中指定位置，在longjmp第二个参数设置返回值，在setjmp()中检查这个返回值。可以做到跨函数跳跃，类似于函数内部的goto。

所以使用alarm() pause() 慢速系统调用三者很有可能产生竞争，Linux中syscall是被中断后自启动的。

13. 使用sigprocmask() 可以用来屏蔽，或者取消屏蔽某个信号，但是如果在sigprocmask()之后调用sleep() 函数，程序进入睡眠，这个期间产生的某个屏蔽信号，他会被投递到这个进程，进行处理！ APUE 10-11

14. 使用sigaction(int signum, const struct sigaction *act,struct sigaction *oldact)对于信号进行处理，struct sigaction下的成员变量sa_flags可以定义各种中断的动作，包括被中断的系统调用是否会重启（SA_INTERUPT）还有信号处理函数只执行一次后复位等（SA_RESETHAND）默认sigaction()函数不再重启被中断的系统调用。

 15. 使用int sigsuspend(const sigset_t *mask)函数可以挂起当前进程，但是当进程收到mask以外的信号并从中断处理函数返回，那么进程从这个函数返回！mask中的信号，进程会屏蔽掉[4]。

16. sleep() 函数与alarm()函数混用，实现需要依赖于具体实现。

17. SIGSTOP、SIGCONT不允许被接管，如果我们需要在SIGSTOP后自定义一些操作，那么我们可以自定义一个信号和信号处理函数。只要跳转到信号处理函数，那么就可以阻止进程访问错误内存地址，进而可以进行一些处理。

 

 

参考：

[1] [http://www.cnblogs.com/mickole/p/3187770.html](http://www.cnblogs.com/mickole/p/3187770.html)

[2] [http://www.man7.org/linux/man-pages/man3/getpwnam.3.html](http://www.man7.org/linux/man-pages/man3/getpwnam.3.html)

[3] [http://blog.csdn.net/feiyinzilgd/article/details/5811157](http://blog.csdn.net/feiyinzilgd/article/details/5811157)

[4] [http://blog.sina.com.cn/s/blog_6af9566301013xp4.html](http://blog.sina.com.cn/s/blog_6af9566301013xp4.html)