---
layout: post
title: "MCA 中的 Monarch’s region"
categories: linux
tags: MCA,Monarch
---
MCA 中的 Monarch’s region
=======================
在do_machine_check()中，扫描banks的前后存在mce_start() 与 mce_end() 。这两个函数可以使得原有kernel中cpus 从并行执行转变为串行执行，先进入这个mce_start()函数的cpu便开始同步，这个函数主要等待所有的cpu都进入mce_start()后，才开始逐个扫描banks。

首先我们必须知道do_machine_check()中声明的order,no_way_out含义，order主要用来标示所有CPU进入handler的顺序。

__no_way_out 大于0时，意味着kernel无法找到安全的方式恢复MCE（初始当前cpu no_way_out=0），而且会在进入mce_start()前首先通过mce_no_way_out()赋值一次no_way_out，判断出一次panic，当前cpu的no_way_out会被赋值为1。global_nwo标示一个全局的值，每个cpu都会有一个no_way_out，而global_nwo只有一个。____

<pre><code>
void do_machine_check(struct pt_regs *regs, long error_code)
{
... 
        int order;
        int no_way_out = 0; 
....
        no_way_out = mce_no_way_out(&m, &msg, valid_banks, regs);
 
        order = mce_start(&no_way_out);
        for (i = 0; i < cfg->banks; i++) {
             .... 
         }
  
        if (!no_way_out)
             mce_clear_state(toclear);
  
        if (mce_end(order) less 0)
             no_way_out = worst >= MCE_PANIC_SEVERITY;
....
</code></pre>

根据代码，我们看到每个cpu都有超时限制，并会将当前的no_way_out加到全局global_nwo上（最后退出君主区域，正常系统应该为0，如果不为0，意味着有panic事件，总的来说global_nwo就是为了维持no_way_out的全局一致性），而mce_callin初始为0，首先到达此处的cpu将order+1。然后等待其他cpu自增，直到等于当前系统cpu的总数。如果等待超时，意味着出现不确定的问题，然后会在

<pre><code>
if (mce_end(order) less 0)
        no_way_out = worst >= MCE_PANIC_SEVERITY;
</code></pre>

直接赋值将no_way_out = 1，然后panic。

所有cpu都进入到mce_start()后，判断order（order=1的是Monarch ，order其他的是仆人，听命于Monarch），order=1时，这个君主将mce_executing置1，其他的cpu等待。

<pre><code>
static int mce_start(int *no_way_out)
{
        int order;
        int cpus = num_online_cpus();
        u64 timeout = (u64)mca_cfg.monarch_timeout * NSEC_PER_USEC;
 
        if (!timeout)
                return -1;
 
        atomic_add(*no_way_out, &global_nwo);
 
        smp_wmb();
        order = atomic_inc_return(&mce_callin);
 
        while (atomic_read(&mce_callin) != cpus) {
                if (mce_timed_out(&timeout)) {
                        atomic_set(&global_nwo, 0);
                        return -1;
                }
                ndelay(SPINUNIT);
        }
 
        smp_rmb();
 
        if (order == 1) {
                /*
                 * Monarch: Starts executing now, the others wait.
                 */
                atomic_set(&mce_executing, 1);
        } else {
                while (atomic_read(&mce_executing) [less] order) {
                        if (mce_timed_out(&timeout)) {
                                atomic_set(&global_nwo, 0);
                                return -1;
                        }
                        ndelay(SPINUNIT);
                }
        }
 
        *no_way_out = atomic_read(&global_nwo);
        return order;
}
</code></pre>

___扫描完君主的banks后，君主cpu进入mce_end(),mce_executing增加，君主cpu等待其他仆人cpu扫描banks，直到所有的mce_executing都执行完毕（典型的cpu同步操作），此刻mce_executing==order==cpu的个数，最后由君主cpu再次确认no_way_out的值，我们可以认为no_way_out就是panic事件的个数。（此处我们要将理解为每个cpu都有一份独立的order，但是所有cpu靠原子mce_executing来做到cpu的同步。）___

在int mce_end(int order)中，仆人cpu直接等待君主cpu的裁决（也就是return），然后君主cpu进入mce_reign(),这个函数就是根据所有cpu扫描的结果，做一次全面的统计，统计出global_worst，然后返回0，如果order出错，直接返回-1。

<pre><code>
static int mce_end(int order)
{
        int ret = -1;
        u64 timeout = (u64)mca_cfg.monarch_timeout * NSEC_PER_USEC;
 
        if (!timeout)
                goto reset;
        if (order [less] 0)
                goto reset;
 
        atomic_inc(&mce_executing);
 
        if (order == 1) {
 
                int cpus = num_online_cpus();
 
                while (atomic_read(&mce_executing) [less and equal] cpus) {
                        if (mce_timed_out(&timeout))
                                goto reset;
                        ndelay(SPINUNIT);
                }
 
                mce_reign();
                barrier();
                ret = 0;
        } else {
 
                while (atomic_read(&mce_executing) != 0) {
                        if (mce_timed_out(&timeout))
                                goto reset;
                        ndelay(SPINUNIT);
                }
                return 0;
        }
reset:
        atomic_set(&global_nwo, 0);
        atomic_set(&mce_callin, 0);
        barrier();
 
        atomic_set(&mce_executing, 0);
        return ret;
}
</code></pre>

退出君主区域，也就是MCA裁决当前系统应该采取哪种策略，是kill当前进程还是panic系统等，详细请看

[do_machine_check() 函数分析](http://lzz5235.github.io/2015/07/20/do_machine_check.html)