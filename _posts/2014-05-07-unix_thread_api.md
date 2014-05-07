---
layout: post
title: "Unix_Thread_API备注"
categories: [Linux]
tags: [Web,Server]
---
Unix Thread API备注
=====================
这是最近读unix pthread and process使用的注意事项。

fork（）可以创建子进程，这个函数返回两次，通过返回值来判断是父进程还是子进程。fork时子进程获得父进程数据空间、堆和栈的复制，所以变量的地址（当然是虚拟地址）也是一样的。但是需要了解的是我们要分开虚拟地址和物理地址，两个进程虚拟地址可以相同，但是物理地址不同。

> ___出于效率考虑，linux中引入了“写时复制“（Copy on Write）技术，也就是只有进程空间的各段的内容要发生变化时，才会将父进程的内容复制一份给子进程。___

但是会把父子共享的页面标记为“只读”（类似mmap的private的方式），如果父子进程一直对这个页面是同一个页面，知道其中任何一个进程要对共享的页面“写操作”，这时内核会复制一个物理页面给这个进程使用，同时修改页表。而把原来的只读页面标记为“可写”，留给另外一个进程使用。

___同时注意stdin stdout是行缓冲，也就意味着子进程会复制父进程输入输出的内容。fork（）函数前的printf（）部分也会复制！__

<pre><code>
while (1) {
    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
    if (Fork() == 0) {
        Close(listenfd); /* Child closes its listening socket */
        echo(connfd);    /* Child services client */
        Close(connfd);   /* Child closes connection with client */
        exit(0);         /* Child exits */
    }
    Close(connfd); /* Parent closes connected socket (important!) */
    }
</code></pre>

这个就是返回子进程要先关闭父进程的监听套接字，因为子进程复制了父进程的变量，套接字也是一个道理！

pthread_t pthread_self(void); 函数作用：获得线程自身的ID。
 

void pthread_exit(void *thread_return) 终止所有线程，父进程先等待子线程退出，然后父进程退出
 

void pthread_cancel(tid)退出当前线程
 

int pthread_join(tid,void **return)
 

int pthread_detach(tid)
编程当中可以用pthread_self返回当前线程的tid。

创建一个线程默认的状态是joinable, 如果一个线程结束运行但没有被join,则它的状态类似于进程中的Zombie Process,即还有一部分资源没有被回收（退出状态码），所以创建线程者应该调用pthread_join来等待线程运行结束，并可得到线程的退出代码，回收其资源（类似于wait,waitpid)
但是调用pthread_join(pthread_id)后，如果该线程没有运行结束，调用者会被阻塞，在有些情况下我们并不希望如此.

__比如在Web服务器中当主线程为每个新来的链接创建一个子线程进行处理的时候，主线程并不希望因为调用pthread_join而阻塞（因为还要继续处理之后到来的链接），这时可以在子线程中加入代码pthread_detach(pthread_self())__

或者父线程调用

__pthread_detach(thread_id)（非阻塞，可立即返回）这将该子线程的状态设置为detached,则该线程运行结束后会自动释放所有资源。__

在web服务器里面，线程有同步问题。

<pre><code>
void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots);                          /* Wait for available slot */
    P(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear)%(sp->n)] = item;   /* Insert the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->items);                          /* Announce available item */
}
 
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items);                          /* Wait for available item */
    P(&sp->mutex);                          /* Lock the buffer */
    item = sp->buf[(++sp->front)%(sp->n)];  /* Remove the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->slots);                          /* Announce available slot */
    return item;
}
....................
for (i = 0; i < NTHREADS; i++)  
    Pthread_create(&tid, NULL, thread, NULL);//创建了大量的thread线程，但是sbuf_remove(&sbuf)相当于V（）操作，如果临界值小宇0，那么阻塞！
 
    while (1) { 
    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
    sbuf_insert(&sbuf, connfd); /* 这里面包含了锁，涉及的PV操作 ，相当于P()，临界值＋1*/
    }
}
 
void *thread(void *vargp) 
{  
    Pthread_detach(pthread_self()); 
    while (1) { 
    int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */
    echo_cnt(connfd);                
    Close(connfd);
    }
}
</code></pre>
