---
layout: post
title: "VFS Data Structure关系(3) – 跨文件系统的文件操作分析"
categories: linux
tags: VFS,systemcall
---
VFS Data Structure关系(3) – 跨文件系统的文件操作分析
=====================

通过对VFS 以及文件系统的分析，我们现在来分析一下跨文件系统的文件传输

http://lzz5235.github.io/2014/10/25/vfs-data-structure.html

http://lzz5235.github.io/2014/10/28/vfs-data-structure2.html

比如从一个floppy传输到harddisk（ext2——>ext4）

首先我们明确一个概念：一切皆文件！

在linux中，无论是普通的文件，还是特殊的目录、设备等，VFS都将它们同等看待成文件，通过统一的文件操作界面来对它们进行操作。操作文件时需先打开；打开文件时，VFS会知道该文件对应的文件系统格式；当VFS把控制权传给实际的文件系统时，实际的文件系统再做出具体区分，对不同的文件类型执行不同的操作。

原理：将ext2格式的磁盘上的一个文件a.txt拷贝到ext4格式的磁盘上，命名为b.txt。这包含两个过程，对a.txt进行读操作，对b.txt进行写操作。读写操作前，需要先打开文件。由前面的分析可知，打开文件时，VFS会知道该文件对应的文件系统格式，以后操作该文件时，VFS会调用其对应的实际文件系统的操作方法。所以，VFS调用vfat的读文件方法将a.txt的数据读入内存；在将a.txt在内存中的数据映射mmap()到b.txt对应的内存空间后，VFS调用ext4的写文件方法将b.txt写入磁盘；从而实现了最终的跨文件系统的复制操作。

关于mmap()使用
fs/open.c
<pre><code>
long do_sys_open(int dfd, const char __user *filename, int flags, umode_t mode)
{
    struct open_flags op;
    int fd = build_open_flags(flags, mode, &op);
    struct filename *tmp;
 
    if (fd)
        return fd;
 
    tmp = getname(filename);
    if (IS_ERR(tmp))
        return PTR_ERR(tmp);
 
    fd = get_unused_fd_flags(flags);
    if (fd >= 0) {
        struct file *f = do_filp_open(dfd, tmp, &op);
        if (IS_ERR(f)) {
            put_unused_fd(fd);
            fd = PTR_ERR(f);
        } else {
            fsnotify_open(f);
            fd_install(fd, f);
        }
    }
    putname(tmp);
    return fd;
}
</code></pre>
下面我们来分析下linux-3.14.8的内核(Not Finished!)

<pre><code>
sys_open()
    |
    |------do_sys_open()
    |           |
    |           |------get_unused_fd_flags()
                |
                |------do_filp_open()
                            |
                            |----path_openat()
                            |       |
                            |       |----link_path_walk()
                            
</code></pre>

