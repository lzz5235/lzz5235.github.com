---
layout: post
title: "浅谈APIC timer"
categories: linux
tags: linux,apci,timer
---
最近在写毕业论文，博客比较荒废，下面我们来谈一下APIC timer吧。

因为我的毕业论文涉及benchmark，所以测试性能与时间紧密相关，我必须调整操作系统的时钟频率，操作本身计时器不够精确，所以必须手动调整Local APIC时钟触发时间中断间隔（每秒触发多少时间中断）。硬件实现上每个Local APIC连接一个CPU core。

所以在编写APIC timer驱动的时候，提供了2-3种模式来实现，第一种和第二种分别是周期模式和one-shot模式，被所有的Local APIC支持，第三种叫TSC-Deadline mode，是最近的CPU型号支持，比如在MCA的君主模式中，我看到时间戳通常使用第三种定时器模式。

###周期模式

驱动可以通过设置初始count值作为发生时间中断的依据，每当这个count值减为0，就会产生一次timer IRQ，然后重新设置为初始count值，重新开始自减，所以这种模式下Local APIC产生中断的间隔取决于初始count值，而自减频率与CPU的外频和步长（divided by the value）相关，步长值存储在“Divide Configuration Register” 寄存器中。

举个例子2.4GHz CPU拥有外频800MHZ，如果步长为4，初始count值为123456，那么Local APIC将以200MHZ的速率自减couns值，每个timer IRQ中断间隔为617.28us，时间中断频率则是1620.01Hz。

###one-shot模式

这个模式和周期模式很类似，不同的是他不会重置初始count值，也就是说驱动必须亲自重置这个count值，如果内核想要更多的IRQ中断。这种模式的优势是驱动可以更加精确地控制timer IRQ的产生。举个例子，内核切换进程时可以依赖新进程的优先级（Priority），这样可以动态改变IRQ时钟频率。一些内核可以使用这种方式实现更加精确地timer服务。

比如当前运行的进程应该抢先1234纳秒，而同时一个睡眠进程要在333纳秒后醒来，时间中断将会在44444纳秒后到来。那么初始count值可以设置为333纳秒，那是内核发生Timer IRQ，内核知道当前进程还有901纳秒被调度，同时下次Timer IRQ将在441111纳秒后到来。

这种模式的缺点在于很难跟踪实时进程，并且需要避免竞争条件，特别是新的count值在旧count值结束前被设置。

###TSC-Deadline 模式

这种模式和前两种完全不同，他不是使用外频/总线频率降低count数值，而是通过软件设置deadline，当CPU时间戳计数大于deadline时，Local APIC产生timer IRQ。这种方式相比one-shot模式，Timer IRQ可以有更高的精度，因为时间戳是以CPU主频的方式自增，这个明显高于外频，也避免了竞争条件。

 

###参考：

[http://wiki.osdev.org/APIC_timer](http://wiki.osdev.org/APIC_timer)

[http://www.cs.columbia.edu/~junfeng/11sp-w4118/lectures/trap.pdf](http://www.cs.columbia.edu/~junfeng/11sp-w4118/lectures/trap.pdf)