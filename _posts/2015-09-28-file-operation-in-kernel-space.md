---
layout: post
title: "File Operation in kernel space"
categories: linux 
tags: driver
---
在内核中对文件进行读写
=====================
我们知道在用户态下，使用各种文件的系统调用即可对文件进行读写操作，open()、write()、read()等。这些调用最后都会通过内核的VFS模型，调用到设备驱动函数，这里我们可以简单看一下open()、write()、read()驱动函数接口：

<pre><code>
static int xxx_open(struct inode *inode, struct file *file);
static int xxx_release(struct inode *inode, struct file *file);
static ssize_t xxx_read(struct file *filp, char __user *ubuf,
                                  size_t usize, loff_t *off);
static unsigned int xxx_poll(struct file *file, poll_table *wait);
static long xxx_ioctl(struct file *f, unsigned int cmd,
                                  unsigned long arg);
ssize_t xxx_write(struct file *filp, const char __user *ubuf,
                          size_t usize, loff_t *off)
</code></pre>

实际上在内核驱动调用的函数传入的参数远比我们在用户态下看到的复杂的多，那么问题来了：如果在内核态下对文件进行读取？当然我们不能在内核中使用syscall了，这里我们有两种方式：1.将全部的驱动函数导出 2.使用VFS在内核态中的接口。

将全部的驱动函数导出的方式虽然可行，但是这样做等于将函数暴露在全局，不利于封装，不推荐使用这种方式。第二种是我们推荐的方式，内核为开发者提供了filp_open(),filp_close(),vfs_read(),vfs_write(),vfs_fsync()接口，我们只需要调用这些接口即可在内核态下对文件进行操作。

首先我们要包含头文件：

<pre><code>
#include < linux/fs.h >
#include < asm/segment.h >
#include < asm/uaccess.h >
#include < linux/buffer_head.h >
</code></pre>

当然了，很多时候我们在内核层下封装了这些接口，使其可以像用户态下open那般简单易用。不过我们要注意open的返回值不再是fd，而是struct file *类型的指针！而path就是路径，flag是读写权限。

<pre><code>
struct file* file_open(const char* path, int flags, int rights) {
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;
 
    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if(IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}
</code></pre>

关闭一个文件：

<pre><code>
void file_close(struct file* file) {
    filp_close(file, NULL);
}
</code></pre>

读取文件的封装接口参数比较多，第一个参数是文件指针，第二个是偏移量，第三个是buffer，第四个是读取的大小。与用户态下的read()类似！

<pre><code>
int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;
 
    oldfs = get_fs();
    set_fs(get_ds());
 
    ret = vfs_read(file, data, size, &offset);
 
    set_fs(oldfs);
    return ret;
}   
</code></pre>

写数据到文件中：

<pre><code>
int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;
 
    oldfs = get_fs();
    set_fs(get_ds());
 
    ret = vfs_write(file, data, size, &offset);
 
    set_fs(oldfs);
    return ret;
}
</code></pre>

立即回写到磁盘，同步文件：

<pre><code>
int file_sync(struct file* file) {
    vfs_fsync(file, 0);
    return 0;
}
</code></pre>


[http://stackoverflow.com/questions/1184274/how-to-read-write-files-within-a-linux-kernel-module](http://stackoverflow.com/questions/1184274/how-to-read-write-files-within-a-linux-kernel-module)

[https://en.wikipedia.org/wiki/Virtual_file_system](https://en.wikipedia.org/wiki/Virtual_file_system)