---
layout: post
title: "编写Share Object Library"
categories: [Linux]

tags: [C/C++,Share Object Library]
 


---
编写Share Object Library
===========================
最近要编写供自己使用的library，library是以.so结尾的文件。类似于windows下面的dll文件。
在编写so文件之前，我们要先熟悉ldd命令与ldconfig命令
在上一篇博客中我写道ld主要是为了从library中寻找特定的库函数并加载到ld.so.cache与GOT表中，ldconfig在/sbin/下。
ldconfig为在命令行中说明的目录或文件/etc/ld.so.config中指定的目录或一些可信任的目录 (象/usr/lib, /lib)中的最新的动态连接库创建必要的连接和绶存。这些绶存起来的数据会被动态连接器ld.so 或 ld-linux.so所使用。ldconfig会检查它所遇到的动态库文件的名称与版本号，以决定那些动态库的连接要进行更新。

静态库链接时搜索路径顺序：

> 1. ld会去找GCC命令中的参数-L
> 2. 再找gcc的环境变量LIBRARY_PATH
> 3. 再找内定目录 /lib /usr/lib /usr/local/lib 这是当初compile gcc时写在程序内的

动态链接时、执行时搜索路径顺序:

> 1. 编译目标代码时指定的动态库搜索路径
2. 环境变量LD_LIBRARY_PATH指定的动态库搜索路径
3. 配置文件/etc/ld.so.conf中指定的动态库搜索路径
4. 默认的动态库搜索路径/lib
5. 默认的动态库搜索路径/usr/lib

有关环境变量：
LIBRARY_PATH环境变量：指定程序静态链接库文件搜索路径

LD_LIBRARY_PATH环境变量：指定程序动态链接库文件搜索路径

其中ldd是一个寻找程序依赖库的命令。比如hello world中。依赖了下列的三个动态库。

<pre><code>
[lzz@localhost day4]$ ldd hello
    linux-vdso.so.1 =>  (0x00007fffaa9a3000)
    libc.so.6 => /lib64/libc.so.6 (0x0000003cf1a00000)
    /lib64/ld-linux-x86-64.so.2 (0x0000003cf1600000)
[lzz@localhost day4]$ 
</code></pre>

说完了上面的命令，我开始编写自己的so。

![](/assets/pic/Diagram_dl.png)

编写so文件就是将这些函数放入一个.c文件中。

<pre><code>
libstdio.c
#include <stdio.h>                                                                                                                     
#include <stdlib.h>
 
int lib_print(const char *str)
 
{
    printf("My  First Print str %s\n",str);
    return 0;
}
[lzz@localhost day5]$ gcc -O2 -fPIC -shared libstdio.c -o libstdio.so.0
</code></pre>

最主要的是GCC命令行的一个选项:
-shared 该选项指定生成动态连接库（让连接器生成T类型的导出符号表，有时候也生成弱连接W类型的导出符号），不用该标志外部程序无法连接。相当于一个可执行文件
-fPIC：表示编译为位置独立的代码，不用此选项的话编译后的代码是位置相关的所以动态载入时是通过代码拷贝的方式来满足不同进程的需要，而不能达到真正代码段共享的目的。

-O2
Optimize even more. GCC performs nearly all supported optimizations that do not involve a space-speed tradeoff. As compared to -O, this option increases both compilation time and the performance of the generated code.
编译完会生成一个libstdio.so.0文件。
然后我们使用dlopen函数族打开这些so函数。

参考 http://linux.die.net/man/3/dlopen

<pre><code>
dlopen.c
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
 
int main(int argc, char **argv) 
{
    void *handle;
    int (*print)(const char *);
    char *error;
 
    handle = dlopen ("/home/lzz/dslab/day5/libstdio.so.0", RTLD_LAZY);
    if (!handle) 
    {
        fputs (dlerror(), stderr);
        exit(1);
    }
    print = dlsym(handle, "lib_print");
    if ((error = dlerror()) != NULL)
    {
        fputs(error, stderr);
        exit(1);
    }
 
    printf ("%d\n", (*print)("Test"));
    dlclose(handle);
}
[lzz@localhost day5]$gcc -O2 dlopen.c -o dlopen -ldl
[lzz@localhost day5]$ ./dlopen 
My  First Print str Test
0
[lzz@localhost day5]$ 
</code></pre>

####另外我想起这周的说起的gcc一个优化参数：
####-g3 为了在debug中找到更多的问题，使用-g3可以包含更多的预处理。

####gcc -g3 –verbose-asm -Wall -alh code.c 这个可以使得asm代码更加具有可读性