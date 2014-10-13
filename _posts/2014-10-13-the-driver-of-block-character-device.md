---
layout: post
title: "The Driver of Block Character Device"
categories: linux
tags: driver,charater device
---
具有阻塞操作的字符设备驱动
==========================
[http://lzz5235.github.com/2014/10/13/the-driver-of-character-device.html](http://lzz5235.github.com/2014/10/13/the-driver-of-character-device.html)
在上面这篇文章里面，我们做的是一个简单的非阻塞操作的字符设备，也就是说对于这个设备的操作要么放弃，要么不停的轮询，直到操作可以进行下去。

而支持阻塞设备的字符操作，我们可以实现读操作或者写操作的睡眠。这就是我们要完成的操作。

关于程序的框架，与之前的其实是类似的，比如我们在全局设定一个等待队列头。

<pre><code>
#define MYCDEV_SIZE 100
#define DEVICE_NAME "lzz_block_cdev"
 
static char globalmem[MYCDEV_SIZE];
static wait_queue_head_t rdwait;
static wait_queue_head_t wrwait;
static struct semaphore mutex;
 
static int len;
ssize_t myblock_read(struct file*,char*,size_t count,loff_t*);
ssize_t myblock_write(struct file*,char*,size_t count,loff_t*);
ssize_t mycdev_open(struct inode *inode, struct file *fp);
ssize_t mycdev_release(struct inode *inode, struct file *fp);
</code></pre>

在模块初始化中，首先要初始化mutex，rdwait，wrwait。

<pre><code>
static int __init mycdev_init(void)
{
    int ret;
 
    printk("myblock module is working..\n");
 
    ret=register_chrdev(MYCDEV_MAJOR,DEVICE_NAME,&fops);
    if(ret<0)
    {
        printk("register failed..\n");
        return 0;
    }
    else
    {
        printk("register success..\n");
    }
    sema_init(&mutex,1);
    init_waitqueue_head(&rdwait);
    init_waitqueue_head(&wrwait);
 
    return 0;
}
</code></pre>

先创建一个代表当前进程的等待队列结点wait，并把它加入到读等待队列当中。

当共享数据区的数据长度为0时，就阻塞该进程。因此，在循环中，首先将当前进程的状态设置TASK_INTERRUPTIBLE。

然后利用schedule函数进行重新调度，此时，读进程才会真正的睡眠，直至被写进程唤醒。在睡眠途中，如果用户给读进程发送了信号，那么也会唤醒睡眠的进程。

当共享数据区有数据时，会将count字节的数据拷贝到用户空间，并且唤醒正在睡眠的写进程。当上述工作完成后，会将当前进程从读等待队列中移除，并且将当前进程的状态设置为TASK_RUNNING。

关于从全局缓冲区移出已读数据，这里要特别说明一下。这里利用了memcpy函数将以（globalmem+count）开始的（len-count）字节的数据移动到缓冲区最开始的地方。

###signal_pending(current）检查当前进程是否有信号处理，若要处理就返回非0！然后这个函数退出，插入到等待队列，等待下次重新开始执行！

<pre><code>
ssize_t myblock_read(struct file*fp,char*buf,size_t count,loff_t*offp)
{
    int ret;
    DECLARE_WAITQUEUE(wait,current);
 
    down(&mutex);
    add_wait_queue(&rdwait,&wait);
 
    while(len==0)
    {
        __set_current_state(TASK_INTERRUPTIBLE);
        up(&mutex);
        schedule();
        if(signal_pending(current))
        {
            ret=-1;
            goto signal_out;
        }
 
        down(&mutex);
    }
 
    if(count>len)
    {
        count=len;
    }
 
    if(copy_to_user(buf,globalmem,count)==0)
    {
        memcpy(globalmem,globalmem+count,len-count);
        len-=count;
        printk("read %d bytes\n",count);
        wake_up_interruptible(&wrwait);
        ret=count;
    }
    else
    {
        ret=-1;
        goto copy_err_out;
    }
 
copy_err_out:up(&mutex);
signal_out:remove_wait_queue(&rdwait,&wait);
 
    set_current_state(TASK_RUNNING);
    return ret;
}
</code></pre>

写函数的控制流程大致与读函数相同，只不过对应的等待队列是写等待队列。
唤醒后如何执行。无论因哪种方式而睡眠，当读进程被唤醒后，均顺序执行接下来的代码。

<pre><code>
ssize_t myblock_write(struct file*fp,char*buf,size_t count,loff_t*offp)
{
    int ret;
    DECLARE_WAITQUEUE(wait,current);
 
    down(&mutex);
    add_wait_queue(&wrwait,&wait);
 
    while(len==MYCDEV_SIZE)
    {
        __set_current_state(TASK_INTERRUPTIBLE);
        up(&mutex);
        schedule();
        if(signal_pending(current))
        {
            ret=-1;
            goto signal_out;
        }
 
            down(&mutex);
    }
    if(count>(MYCDEV_SIZE-len))
    {
        count=MYCDEV_SIZE-len;
    }
 
    if(copy_from_user(globalmem+len,buf,count)==0)
    {
        len=len+count;
        printk("written %d bytes\n",count);
        wake_up_interruptible(&rdwait);
        ret=count;
    }
    else
    {
        ret=-1;
        goto COPY_ERR_OUT;
    }
 
signal_out:up(&mutex);
COPY_ERR_OUT:remove_wait_queue(&wrwait,&wait);
    set_current_state(TASK_RUNNING);
 
    return ret;
}
</code></pre>

读写函数down操作和add_wait_queue操作交换，可能会造成死锁。

up操作和remove_wait_queue操作交换。如果读进程从内核空间向用户空间拷贝数据失败时，就会从up往后执行。

因为读进程是在获得信号量后才拷贝数据的，因此必须先释放信号量，再将读进程对应的等待队列项移出读等待队列。而当读进程因信号而被唤醒时，则直接跳转到remove_wait_queue操作，并往后执行（仅仅只是睡眠了，wake_up后继续向后执行）。

此时读进程并没有获得信号量，因此只需要移出队列操作即可。如果交换上述两个操作，读进程移出等待队列时还未释放互斥信号量，那么写进程就不能写。而当读进程因信号而唤醒时，读进程并没有获得信号量，却还要释放信号量。

####对于goto操作：就是从Label以后，一直往后执行，并不是执行一条语句而已。

使用模块的方式，上篇博文说的很清楚。
使用的时候：

* 终端输入：cat /dev/blockcdev&；即从字符设备文件中读数据，并让这个读进程在后台执行，可通过ps命令查看到这个进程；

* 中断继续输入：echo ‘I like eating..’ > /dev/blockcdev；即向字符设备文件中写入数据；
