---
layout: post
title: "字符驱动poll函数与select()函数的交互"
categories: linux
tags: poll,select
---
字符驱动poll函数与select()函数的交互
==============
在字符驱动中，我们经常要实现poll()的功能，具体实现在注册到file_operations 的函数中，举个例子

<pre><code>
static unsigned int xxx_poll(struct file *file, poll_table *wait)
{
         poll_wait(file, &mp_chrdev_wait, wait);
         if (rcu_access_index(mplog.next))
                 return POLLIN | POLLRDNORM;
  
         return 0;
}
</code></pre>

我们必须在这个函数中返回POLLIN、POLLOUT等状态，从而我们可以在用户态下使用FD_ISSET()判断数据是否到来。而其中void poll_wait(struct file *filp, wait_queue_head_t *queue, poll_table *wait);它的作用就是把当前进程添加到wait参数指定的等待列表(poll_table)中。需要注意的是这个函数是不会引起阻塞的。

___这里我们实现创建了一个mp_chrdev_wait的等待队列，它会把这个轮训进程放入一个等待队列中，然后这个进程会睡眠（表现在select()上就是阻塞）。当某个条件满足时，唤醒这个等待队列，也就是唤醒了轮训进程，也就是内核通知应用程序（应用程序的select函数会感知），这个时候mask返回值中有数据。然后就会接着select操作。___所以我们要在恰当的位置wake_up_interruptible(&mp_chrdev_wait)

在用户空间中的代码，我们需要使用select()轮训在这个设备上：

<pre><code>
         int register_fd, ret;
         fd_set rds;
  
         register_fd = open(CONFIG_PATH, O_RDWR);
         if (register_fd less 0)
                err("opening of /dev/mplog");
 
         FD_ZERO(&rds);
         FD_SET(register_fd,&rds);
  
  
         while (1) {
                 /*
                  * Proceed with the rest of the daemon.
                 */
                memset(temp, 0, MP_LOG_LEN * sizeof(struct mp));
  
 
                 ret = select(register_fd+1,&rds,NULL,NULL,NULL);
                 if(ret less 0 )
                 {
                         close(register_fd);
                         err("select error!");
                 }
                 if(FD_ISSET(register_fd,&rds))
                        read(register_fd, temp, MP_LOG_LEN * sizeof(struct mp));
.... 
         }
</code></pre>
