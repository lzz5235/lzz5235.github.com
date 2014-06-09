---
layout: post
title: "异常控制流（读书笔记）"
categories: [read]

tags: [Kernel,Interrupt,Read]
 
---
异常控制流（读书笔记）
=====================
今天阅读了一下计算机对于异常处理的章节，我把一些有用的摘出来，方便以后回顾。
异常控制除了中断，还包括system call，我会一直向这篇文章中添加自己读书内容的。

异常表：___当处理器检测到有事件发生时，它会通过跳转表，进行一个间接过程调用（异常），到异常处理程序。__

异常号：系统中可能的某种类型的异常都分配了一个唯一的非负整数的异常号。异常号是到异常表中的索引。

![](/assets/pic/5125.jpg)

一旦硬件触发了异常，异常处理程序则由软件完成。

![](/assets/pic/2046.jpg)

异常的类别——中断、陷阱、故障和终止
处理器注意到中断引脚的电压值变高，就从系统总线读取异常号。    

![](/assets/pic/9921.jpg)

陷阱和系统调用：系统调用是一些封装好的函数，内部通过指令int n实现。陷阱最重要的用途是提供系统调用。系统调用运行在内核模式中，并且可以访问内核中的栈。

* Linux 0-31号中断是对应intel架构师定义的异常，任何IA32都是一样的，32-255是OS自定义的interrupt与trap。

系统调用的参数是通过通用寄存器而不是栈来传递的，如，%eax存储系统调用号，%ebx,%ecx,%edx,%esi,%edi,%ebp最多存储六个参数，%esp不能用，因为进入内核模式后，会覆盖掉它。

高位地址是留给kernel的 ，低位地址留给用户程序的。
私有地址空间

一个进程为每个程序提供它自己的私有地址空间。运行应用程序代码的进程初始时是在用户模式中的。进程从用户模式变为内核模式的唯一方法是通过异常。

* linux提供了/proc文件系统，它允许用户模式进程访问内核数据结构的内容。

上下文切换，调度

上下文切换：操作系统内核使用叫上下文切换的异常控制流来实现多任务。

上下文切换：a）保存当前进程的上下文；b）恢复某个先前被抢占的进程被保存的上下文； c）将控制传递给这个新恢复的进程

fork（）函数
父进程通过调用fork创建一个新的运行子进程：父进程与子进程有相同（但是独立的）地址空间，有相同的文件藐视符集合。

* 回收子进程的两种方法：1,内核的init进程 2,父进程waitpid函数

1）如果父进程没有回收它的僵死子进程就终止了，那么内核就会安排init进城来回收它们。init进程的PID为1，并且是在系统初始化时创建的。

2）一个进程可以通过调用waitpid函数来等待它的子进程终止或停止。
* fork（）调用一次，返回两次。返回0等于是子进程，返回大于0为父进程。并发执行，不能假设先后执行顺序。父子进程有独立的用户空间。

waitpid（）意义在于等待父进程下的子进程运行完毕后，父进程再继续执行。

<pre><code>
#include "csapp.h"
 
/* $begin waitprob1 */
int main() 
{
    int status;
    pid_t pid;
   
    printf("Hello\n");
    pid = Fork();
    printf("%d\n", pid);
    if (pid != 0) {
    if (waitpid(-1, &status, 0) > 0) {
        if (WIFEXITED(status) != 0)
        printf("%d\n", WEXITSTATUS(status));
    }
    }
    printf("Bye\n");
    exit(2);
}
/* $end waitprob1 */
[root@localhost ecf]# gcc -o waitprob1 waitprob1.c csapp.h csapp.c -lpthread
[root@localhost ecf]# ./waitprob1 
Hello
5533
0
Bye
2
Bye
</code></pre>

运行结果含义就是先是父进程，然后是子进程（我们不对父子执行顺序有任何假设），然后父进程等待子进程，子进程pid为0然后打印0 bye 返回值是2，然后父进程回收子进程 打印子进程返回值2 再打印自身的Bye。

execve（）函数在当前进程的上下文总价在运行一个新的程序，他会覆盖当前的地址空间，并没有创建新进程。新进程仍然用的是相同的PID。

信号
=====
信号在Linux系统里占有重要地位，当app发生dump时，会向kernel发送信号。

5SIGCHLD忽略一个子进程停止或终止

> *号码 *名字 *默认行为 *相应事件

> 1 SIGHUP	终止	终端线挂起

> 2	SIGINT	终止	来自键盘的中断

> 3	SIGQUIT	终止	来自键盘的退出

> 4	SIGILL	终止	非法指令

> 9	SIGKILL	终止	杀死程序

> 17	SIGCHLD	忽略	一个子进程停止或终止

发送信号给进程基于进程组的概念。进程组由一个正整数ID标识，每个进程只属于一个进程组。

用 kill 命令向其他进程发送任意信号，给定的PID为负值时，表示发送信号给进程组ID为PID绝对值的所有进程。

进程可以用 kill 函数发送信号给任意进程（包括自己）。

比如我们在terminal中输入kill -9 processpid 可以杀死一个进程， -9 其实就是发送SIGKILL命令。其他的信号也是一样的。

每种信号都有默认行为，可以用 signal 函数修改和信号关联的默认行为__（除 SIGSTOP 和 SIGKILL 外）__：

<pre><code>
#include <signal.h>

typedef void (*sighandler_t)(int);
/** 改变和信号signum关联的行为
 * @return      返回前次处理程序的指针，出错返回SIG_ERR */
sighandler_t signal(int signum, sighandler_t handler);
</code></pre>

参数说明：

signum 信号编号。

handler 指向用户定义函数，也就是信号处理程序的指针。或者为：

> SIG_IGN ：忽略 signum 信号。

> SIG_DFL ：恢复 signum 信号默认行为。

信号处理程序的调用称为捕捉信号，信号处理程序的执行称为处理信号。 signal 函数会将 signum 参数传递给信号处理程序 handler 的参数，这样 handler 可以捕捉不同类型的信号。__不会有重复的信号排队等待。__

signal 的语义和实现有关，最好使用 sigaction 函数代替它。要记住在linux和Solaris对待异常处理的结果是不同的。

对于第三点，___Linux系统会重启系统调用，而Solaris不会。不同系统之间，信号处理语义存在差异。Posix标准定义了 sigaction 函数，使在Posix兼容的系统上可以设置信号处理语义。___

举例！比如在Rio_Read（）程序实现中，read()函数

> 会潜在阻塞进程的慢速系统调用被信号中断后，在信号处理程序返回时不再继续，而返回一个错误条件，并将 errno 设为 EINTR 。
所以我们如果遇到EINTR错误，我们要使用while重启该慢速系统调用！

然后如果我们确实需要信号排队，我们需要使用

SIG_BLOCK，阻塞set中包含的信号。意思是说把set中的信号加到当前的信号掩码中去，新的信号掩码是set和旧信号掩码的并集。

SIG_UNBLOCK，解除set中信号的阻塞，从当前信号掩码中去除set中的信号。

SIG_SETMASK，设置信号掩码，既按照set中的信号重新设置信号掩码。

<pre><code>
int sigemptyset(sigset_t *set)：初始化信号集set使之不包含任何信号，这个函数总是返回0。
int sigaddset(sigset_t *set, int signum)：
//该函数把信号signum加入到信号集set中，需要注意的是这个函数只是修改了set变量本身，并不作其它操作。
//该函数成功操作返回 0，失败返回-1，错误代码设置成EINVAL，表示signum不是有效的信号代码。
int sigprocmask(int how, const sigset_t *set,sigset_t *oldset)，该函数用来检查和改变调用进程的信号掩码

int main(void)
{
       sigset_t block_alarm;
... ...
       sigemptyset(&amp;block_alarm);
       sigaddset(&amp;block_alarm,SIGALRM);
       while(1)
       {
            sigprocmask(SIG_BLOCK,&amp;block_alarm,NULL);
            if(flag)
            {
            ... ...
            flag=0;
            }
            sigprocmask(SIG_UNBLOCK,&amp;block_alarm,NULL);
             ... ...
       }
}

</code></pre>
信号参考 http://blog.chinaunix.net/uid-12461657-id-3188445.html


