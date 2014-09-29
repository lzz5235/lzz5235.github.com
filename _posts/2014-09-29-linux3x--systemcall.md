---
layout: post
title: "在Linux3.X 添加 systemcall"
categories: linux
tags: systemcall
---
在Linux3.X 添加 systemcall
============================
从代码级看syscall的问题

1.寄存器eax，传进来带的是中调用号，ret_from_sys_call()传出的是错误值。

2.由于内核函数是在内核中实现的，因此它必须符合内核编程的规则，比如函数名以sys_开始，函数定义时候需加asmlinkage标识符等。这个修饰符使得GCC编译器从堆栈中取该函数的参数而不是寄存器中。另外，系统调用函数的命名规则都是sys_XXX的形式。

3.不同的syscall，普通用户可用的命令和管理可用的命令分别被存放于/bin和/sbin目录下。

另外比较重要的一个特性就是在Linux 3.X系统中添加新的syscall 这相比于Linux 2.X添加syscall简单不少。Linux 2.X内核网上很多添加调用分析，就不赘述了。

1.定义系统调用服务例程。

<pre><code>
#include <linux/syscall.h>
 
SYSCALL_DEFINE0(mycall)
{
        struct task_struct *p;
        printk(&quot;********************************************\n&quot;);
        printk(&quot;------------the output of mysyscall------------\n&quot;);
        printk(&quot;********************************************\n\n&quot;);
        printk(&quot;%-20s %-6s %-6s %-20s\n&quot;,&quot;Name&quot;,&quot;pid&quot;,&quot;state&quot;,&quot;ParentName&quot;);
        for(p = &amp;init_task; (p = next_task(p)) != &amp;init_task;)
                printk(&quot;%-20s %-6d %-6d %-20s\n&quot;,p-&gt;comm , p-&gt;pid, p-&gt;state, p-&gt;parent-&gt;comm);
        return 0;
}
</code></pre>

以上定义宏SYSCALL_DEFINE0实际上会扩展为：

<pre><code>
asmlinkage int sys_mycall()
{
               struct task_struct *p;
        printk(&quot;********************************************\n&quot;);
        printk(&quot;------------the output of mysyscall------------\n&quot;);
        printk(&quot;********************************************\n\n&quot;);
        printk(&quot;%-20s %-6s %-6s %-20s\n&quot;,&quot;Name&quot;,&quot;pid&quot;,&quot;state&quot;,&quot;ParentName&quot;);
        for(p = &amp;init_task; (p = next_task(p)) != &amp;init_task;)
                printk(&quot;%-20s %-6d %-6d %-20s\n&quot;,p-&gt;comm , p-&gt;pid, p-&gt;state, p-&gt;parent-&gt;comm);
        return 0;
}
</code></pre>

内核在kernel/sys.c文件中定义了SYSCALL_DEFINE0~SYSCALL_DEFINE6等七个提供便利的宏。

2. 为系统调用服务例程在系统调用表中添加一个表项。

编译文件arch/x86/syscalls/syscall_64.tbl，在调用号小于512的部分追加一行：

![](/assets/pic/systemcall.png)

注意：

① 上面的316是紧接着目前已定义系统调用315之后。

② 若为32系统添加系统调用号，在syscall_32.tbl中相应位置追加即可。系统调用在两个文件中的调用号没有必要一样。

③ 不需要像Linux 2.6的内核一样，在<asm/unistd.h>中添加类似于#define __NR_getjiffies 314之类的宏定义了，3.x的内核会自动根据系统调用表的定义生成。

3. 在内核头文件中，添加服务例程的声明。
在include/linux/syscalls.h文件的最后，#endif之前加入系统调用服务例程sys_mysyscall()的声明：

<pre><code>
asmlinkage long sys_mycall(void);
#endif
</code></pre>

剩下就是按照编译内核的方法进行编译好了  
[http://lzz5235.github.io/2013/11/13/linux-kernel.html](http://lzz5235.github.io/2013/11/13/linux-kernel.html)

4.系统调用表产生的过程
之所以Linux3.X与Linux2.X变化这么大。

主要是为了简化添加调用表的过程。

1. 系统调用表的产生过程。
内核开发者是在syscall_64.tbl中声明系统调用号与服务例程的对应关系，以及其ABI，但系统调用表的真正定义是在arch/x86/kernel/syscall_64.c中。

1）arch/x86/kernel/syscall_64.c, arch/x86/kernel/syscall_32.c文件中存放了实际的系统调用表定义，以64位系统为例，其中有如下内容：

<pre><code>
#include &lt;asm/asm-offsets.h&gt;
 
#define __SYSCALL_COMMON(nr, sym, compat) __SYSCALL_64(nr, sym, compat)
 
#ifdef CONFIG_X86_X32_ABI
# define __SYSCALL_X32(nr, sym, compat) __SYSCALL_64(nr, sym, compat)
#else
# define __SYSCALL_X32(nr, sym, compat) /* nothing */
#endif
 
#define __SYSCALL_64(nr, sym, compat) extern asmlinkage void sym(void) ; // 注意，是分号结尾
#include &lt;asm/syscalls_64.h&gt; // 引入系统调用服务例程的声明
#undef __SYSCALL_64
 
#define __SYSCALL_64(nr, sym, compat) [nr] = sym, //注意，是逗号结尾
 
typedef void (*sys_call_ptr_t)(void);
 
extern void sys_ni_syscall(void);
 
const sys_call_ptr_t sys_call_table[__NR_syscall_max+1] = { //系统调用表定义
        /*
         * Smells like a compiler bug -- it doesn't work
         * when the &amp; below is removed.
         */
        [0 ... __NR_syscall_max] = &amp;sys_ni_syscall,
#include &lt;asm/syscalls_64.h; // 系统调用服务例程地址，对应arch/x86/include/generated/asm/syscalls_64.h;
};
</code></pre>

2） arch/x86/syscalls目录中的syscall_64.tbl、syscall_32.tbl文件是系统调用表声明。syscalltbl.sh脚本负责生产syscalls_XX.h文件，由Makefile负责驱动

3） arch/x86/include/generated目录，其中存放根据arch/x86/syscalls目录生成的文件。主要有generated/asm/syscalls_64.h、generated/asm/syscalls_32.h文件，用于生成系统调用表数组。生成的syscalls_64.h内容部分如下：

![](/assets/pic/systemcall2.png)

2. 系统调用号声明头文件的生成(#define0 __NR_xxx之类的信息）。

类似于系统调用表的产生，arch/x86/syscalls/syscallhdr.sh脚本负责generated/uapi/asm/unistd_32.h, generated/uapi/asm/unistd_64.h文件，unistd_XX.h文件又被间接include到asm/unistd.h中，后者最终被include用户空间使用的<sys/syscall.h>文件中(安装之后)。生成的generated/uapi/asm/unistd_64.h部分内容如下

<pre><code>
... ...
#define __NR_sched_getattr 315
#define __NR_mycall 316
 
#endif /* _ASM_X86_UNISTD_64_H */
</code></pre>

注意，这里的unistd.h文件与用户空间使用的文件没有任何关系，后者声明了系统调用包装函数，包括syscall函数等

下面我们来调用这个mycall()

![](/assets/pic/systemcall3.png)

还有一种方式使用内嵌汇编的方式，具体文件在arch/x86/um/shared/sysdep/stub_64.h下面,模拟_syscall0宏调用。

<pre><code>
#define __syscall_clobber "r11","rcx","memory"
#define __syscall "syscall"
 
static inline long stub_syscall0(long syscall)
{
    long ret;
 
    __asm__ volatile (__syscall
        : "=a" (ret)
        : "0" (syscall) : __syscall_clobber );
 
    return ret;
}
</code></pre>

然后编译运行，打印在dmesg里面

[kernel.dmesg](/assets/resource/kernel.dmesg_.txt)