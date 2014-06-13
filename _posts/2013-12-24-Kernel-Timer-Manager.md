---
layout: post
title: "Kernel Timer Manager"
categories: linux

tags: [linux,timer] 
---
Kernel中定时器与时间管理
==========================
linux kernel时钟就是系统定时器以某种频率自行触发，当时钟中断发生时，内核就通过一种特殊的中断处理程序对其进行处理。

利用时钟中断做的事情：

1. 更新系统运行时间 update_wall_time(ticks)
2. 更新实际时间
3. 在SMP系统上，均衡调度各处理器上的运行队列
4. 检查自己时间片是否用尽，耗尽的花，就是用schedule（）调度
5. 更新资源消耗负载，比如top右上方统计值

在include/asm-generic/param.h 规定了节拍值。

比如HZ定义的是1000Hz   也就意味着1/1000 s中断一次，除此之外还有USER_HZ这个是在用户空间下使用的，二者相同。

不同的HZ有不同的优缺点，较高的HZ可以对poll（）select（）超时精度的提高，会对系统性能提高带来极大好处。较高的HZ=1000可以使得一个正在运行的程序即使只有2ms时间片，也可以尽快调度。但是HZ=100的话，程序只有在10ms后才可以调度。

jiffies定义在include/linux/jiffies.h中

<pre><code>
extern unsigned long volatile jiffies
</code></pre>
在32位机上市32位，64位机是64位。有时候我们不care高位，所以我们使用强转。C语言限定符volatile表示jiffies是一个易该变的变量，因此编译器将使对该变量的访问从不通过CPU内部cache来进行。

<pre><code>
* The 64-bit value is not atomic - you MUST NOT read it
 * without sampling the sequence number in jiffies_lock.
* get_jiffies_64() will do this for you as appropriate.
*
extern u64 __jiffy_data jiffies_64;
extern unsigned long volatile __jiffy_data jiffies;
#if (BITS_PER_LONG &lt; 64)
u64 get_jiffies_64(void);
#else
static inline u64 get_jiffies_64(void)
{
   return (u64)jiffies;
}
#endif
</code></pre>

要记住，这个数字只要kernel初始化后，就开始一直自增。所以很有可能溢出，在<linux/jiffies.h>中定义了比较时间的宏，防止了jiffies值得溢出。

全局变量xtime：它是一个timeval结构类型的变量，用来表示当前时间距UNIX时间基准1970－01－01 00：00：00的相对秒数值。结构timeval是Linux内核表示时间的一种格式（Linux内核对时间的表示有多种格式，每种格式都有不同的时间精度），其时间精度是微秒。

<pre><code>
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec {
    __kernel_time_t tv_sec;         /* seconds */
    long        tv_nsec;        /* nanoseconds */
};
#endif
struct timeval {
    __kernel_time_t     tv_sec;     /* seconds */
    __kernel_suseconds_t    tv_usec;    /* microseconds */
};
struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime; /* type of dst correction */
};
</code></pre>

时钟中断服务程序主要是调用do_timer（），首先进行jiffies_64++,然后调用update_process_times()；update_times（）

#####对于实际时间，更新xtime我们要使用seqlock锁。

seqlock的实现思路是，用一个递增的整型数表示sequence。写操作进入临界区时，sequence++；退出临界区时，sequence再++。写操作还需要获得一个锁（比如mutex），这个锁仅用于写写互斥，以保证同一时间最多只有一个正在进行的写操作。

当sequence为奇数时，表示有写操作正在进行，这时读操作要进入临界区需要等待，直到sequence变为偶数。读操作进入临界区时，需要记录下当前sequence的值，等它退出临界区的时候用记录的sequence与当前sequence做比较，不相等则表示在读操作进入临界区期间发生了写操作，这时候读操作读到的东西是无效的，需要返回重试。

seqlock写写是必须要互斥的。但是seqlock的应用场景本身就是读多写少的情况，写冲突的概率是很低的。所以这里的写写互斥基本上不会有什么性能损失。
而读写操作是不需要互斥的。seqlock的应用场景是写操作优先于读操作，

<pre><code>
write_seqlock(&xtime_lock);
..............
write_sequnlock(&xtime_lock)；
//读
do
{
      seq = read_seqbegin(&seq_lock);      // 进入临界区
      do_something();
} while (read_seqretry(&seq_lock, seq)); // 尝试退出临界区，存在冲突则重试
从内核空间取得时间的函数式sys_gettimeofday（）.从用户空间获取时间是gettimeofday（）.
</code></pre>

######定时器也是内核编程中的比较重要的一个项目，timer_list的定义在include/linux/timer.h

<pre><code>
struct timer_list 
{
    struct list_head entry;
    unsigned long expires;
    struct tvec_base *base;
 
    void (*function)(unsigned long);
    unsigned long data;
 
    int slack;
 
#ifdef CONFIG_TIMER_STATS
    int start_pid;
    void *start_site;
    char start_comm[16];
#endif
#ifdef CONFIG_LOCKDEP
    struct lockdep_map lockdep_map;
#endif
};
</code></pre>

使用timer比较简单

1. struct timer_list my_timer;
2. init_timer(&my_timer);
3. 填充my_timer     data，expires = jiffies+delay，function
4. 定义自己的My_timer_function()
5. add_timer(&my_timer)；mod_timer（）；del_timer（）

#####要区分del_timer（）在SMP环境下会出现在别的CPU上运行。用del_timer_sync()解决，但不能再中断上下文中运行。

#####短延迟使用linux/delay.h中的void udelay（）与void mdelay（）.

#####如果长延迟，让任务在我们规定的时刻wakeup就使用schedule_timeout()

参考：
[http://blog.csdn.net/axman/article/details/8484583](http://blog.csdn.net/axman/article/details/8484583]
)

[http://www.ibm.com/developerworks/cn/linux/1307_liuming_linuxtime1/](http://www.ibm.com/developerworks/cn/linux/1307_liuming_linuxtime1/)
