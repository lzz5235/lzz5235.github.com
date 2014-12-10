---
layout: post
title: "C/S模型下Server 中fork()的健壮性"
categories: linux
tags: C/S
---
C/S模型下Server 中fork()的健壮性
================================
C/S模型下Server 下编程非常依赖fork（）子进程去处理具体的业务逻辑。 每当accept()接收到一个TCP连接时，主服务器进程就fork一个子服务器进程。子服务器进程调用相应的函数，通过client_fd（连接套接字）对客户端发来的网络请求进程处理；由于客户端的请求已被子服务进程处理，那么主服务器进程就什么也不做，通过sockfd（监听套接字）继续循环等待新的网络请求。

<pre><code>
..........
while (1) {
    sin_size = sizeof(struct sockaddr_in);
    if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size)) == -1) {
        my_error("accept", errno, __LINE__);
        continue;
    }
 
    if ((pid = fork()) == 0) {
        close(sockfd);
        process_client_request(client_fd);
        close(client_fd);
        exit(0);
    } else if (pid > 0)
        close(client_fd);
    else
        my_error("fork", errno, __LINE__);
}
</code></pre>

每个文件都有一个引用计数，该引用计数表示当前系统内的所有进程打开该文件描述符的个数。套接字是一种特殊的文件，当然也有引用计数。 当fork执行后，由于子进程复制了父进程的资源，所以子进程也拥有这两个套接字描述符，则此时sockfd和client_fd的引用计数都为2。只有当子进程处理完客户请求时，client_fd的引用计数才由于close函数而变为0。 但是这里存在一个严重的问题：如果客户端意外退出，就会导致server子进程成为僵尸进程。我们设想如果server非常繁忙，就会导致system出现大量的zombie进程！system会应该耗尽系统资源而宕机！ 这是因为

当一个子进程先于父进程结束运行时，它与其父进程之间的关联还会保持到父进程也正常地结束运行，或者父进程调用了wait才告终止。
子进程退出时，内核将子进程置为僵尸状态，它只保留最小的一些内核数据结构，以便父进程查询子进程的退出状态。
进程表中代表子进程的数据项是不会立刻释放的，虽然不再活跃了，可子进程还停留在系统里，因为它的退出码还需要保存起来以备父进程中后续的wait调用使用。
所以我们要处理zombie进程！ 

两种方式：

* 调用wait或者waitpid函数查询子进程退出状态，此方法父进程会被挂起。

* 如果不想让父进程挂起，可以在父进程中加入一条语句：signal(SIGCHLD,SIG_IGN);表示父进程忽略SIGCHLD信号，该信号是子进程退出的时候向父进程发送的。
当子进程终止时会给父进程发送SIGCHLD信号，因此我们可以利用信号处理函数捕获这个信号并对僵死进程进行处理。我们知道在父进程中调用wait函数可以防止先于父进程终止的子进程编程僵死进程

<pre><code>
void sig_zchild(int signo)
{
    pid_t pid;
    int stat;
 
    pid = wait(&stat);
        printf("child %d terminated\n", pid);
    return;
}
</code></pre>

修改服务器程序，在accept函数调用之前调用signal函数：

<pre><code>
    if(listen(sockfd, BACKLOG) == -1) {
 
        printf("listen error!\n");
        exit(1);
    }
 
    if (signal(SIGCHLD, sig_zchild) == SIG_ERR) {
        printf("signal error!\n");
        exit(1);
    }
 
    while (1) {
 
        sin_size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr,
&sin_size)) == -1) {
 
            printf("accept error!\n");
            continue;
        }
        …… ……
    }//while
</code></pre>

但是这个程序仍存在一个问题：当多个子进程同时退出时，会导致父进程无法同时处理SIGCHILD信号，导致有部分子进程zombie。

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

使用while可以等待SIGCHILD信号，直到处理完成！ 信号在内核中也是存放在队列中！
 

参考：[http://www.cnblogs.com/mickole/p/3187770.html](http://www.cnblogs.com/mickole/p/3187770.html)
