---
layout: post
title: "SystemTap Kprobe原理"
categories: [linux]

tags: [Debug,Gdb] 


---
SystemTap Kprobe原理
====================
想写这篇文章好长时间了，一直没有来得及总结，今天我把这个坑填上！

Systemtap是一种动态调试内核的工具，可以极大地方便内核开发人员对于内核的调试，过去，内核想要调试，必须在源码中打入print（）然后进行编译，安装内核重启，这个导致内核调试复杂。

现在我们可以使用systemtap中的kprobe编写脚本，然后使用stap命令进行编译，即可对内核中的变量函数进行调试。

Systemtap 是基于kprobes 机制实现探测的，即最终使用了kprobe 提供的接口。从kprobe原理可知，对于每一个探针来说，用户需要定义探测点和相应的处理函数。探测点是指kprobe 机制中的被探测函数或者指令地址（亦可称为内核事件），但对于Systemtap 来讲，用户可以指定源码文件、源代码中的某一行，用户也可以指定一个异步事件，比如周期性的定时器。除了Kprobes 之外，还使用了kernel markers 技术，现已被tracepoints 技术替代。

Systemtap 的基本处理流程如图所示，该过程涉及5 个阶段，并且包括3 个具有交互关系的实用程序(stap、staprun、stapio)。

输入：

▶ script.stp –> 普通用户编写

▶ tapset(由脚本语言或C 语言编写) –> 内核开发者编写

▶ DWARF(内核编译时产生的调试信息) –> 内核编译时产生，这里我们要声明一下产生debuginfo的前提是在编译kernel时候勾选debuginfo选项

▶ 运行时库

输出：内核模块

![](/assets/pic/4058.png)

![](/assets/pic/4227.png)

从上面的图，我们可以清楚地看到kprobe的五个阶段：

> Pass 1(parse)：stap 将脚本转换为语法分析树，并检查语法错误。

> Pass 2(elaborate)：1) 从脚本库中添加相应的代码；2) 从当前正在运行的内核的调试信息中获取探测点以及变量的位置。

> Pass 3(translate)：将第二阶段的输出翻译成C 文件，在该阶段Systemtap 将根据需要增加必要的锁和安全检查代码。

> Pass 4(Build)：该阶段是“stap”的最后一步，该阶段主要是构造相应的内核模块，同时需要链接相应的运行时库（runtime libraries）。内核模块的后缀名都是以.ko结尾。

> Pass 5(Execute)：当第四阶段完成之后stap 会生成最终可执行的内核模块，此时stap 也就完成了自己的任务，并将控制权交给Systemtap 的其他两个实用程序：staprun(/usr/bin, 通常可stap 在同一个目录中) 和stapio（/usr/libexec/systemtap），它们负载加载内核模块以及输出探测信息等。

这里面我要说明一下/usr/bin/staprun可以用户调用，/usr/libexec/systemtap/stapio无法用户调用！

另外我从一个论文里看到一幅特别好的示例图，可以给大家分享一下！

![](/assets/pic/4524.png)

这幅图，很清晰的标出了每个阶段链接了哪些库，对我们理解systemtap原理有很大的帮助！

Kprobe的原理其实和GDB原理很像，都是调用INT 3中断然后跳转到特定的arch/x86/kernel/kprobes.c中的handler

![](/assets/pic/0006.png)

