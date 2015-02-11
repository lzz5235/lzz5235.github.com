---
layout: post
title: "__schedule() Analyzation"
categories: linux
tags: schedule
---
__schedule()调度分析
==================
主实现代码：[http://lxr.free-electrons.com/source/kernel/sched/core.c#L2765](http://lxr.free-electrons.com/source/kernel/sched/core.c#L2765)

###调度这一块，因为存在很多的调度policy，kernel为了分离mechanism与具体policy，在__schedule()中实现task的切换，具体policy在pick_next_task() 中实现。

内核中对进程调度的方法有两种，其一为周期性调度器（generic scheduler），它对进行进行周期性的调度，以固定的频率运行；其二为主调度器（main scheduler），如果进程要进行睡眠或因为其他原因主动放弃CPU，那么就直接调用主调度器。

其中，主调度器是__schedule() ,而周期性调度器是void scheduler_tick(void)。这个函数负责每个rq的平衡，保持每个cpu都有task可以运行，这个程序由timer调度。[http://lxr.free-electrons.com/source/kernel/sched/core.c#L2524](http://lxr.free-electrons.com/source/kernel/sched/core.c#L2524)

__schedule（）是调度的核心函数，在这个函数里面是主要是从rq队列中，选择进程。除了切换上下文状态，还要使用 pick_next_task() 使用这个选择下一个进程,具体到使用哪种调度policy都在这个struct sched_class结构体里保存着。

目前kernel在SMP环境下使用的调度算法是CFS算法。具体我们先来看pick_next_task()函数。
我们发现具体的policy在fair_sched_class 定义，GNU C的语法就是用C 的strut来模拟C++的class方式，然后在fair.c中定义了众多的函数，这种方式就是一种钩子函数。具体CFS策略这里不再细讲，之后我会专门来分析CFS调度算法。

<pre><code>	
2692 static inline struct task_struct *
2693 pick_next_task(struct rq *rq, struct task_struct *prev)
2694 {
2695         const struct sched_class *class = &fair_sched_class;
2696         struct task_struct *p;
2697
2698         /*
2699          * Optimization: we know that if all tasks are in
2700          * the fair class we can call that function directly:
2701          */
2702         if (likely(prev->sched_class == class &&
2703                    rq->nr_running == rq->cfs.h_nr_running)) {
2704                 p = fair_sched_class.pick_next_task(rq, prev);
2705                 if (unlikely(p == RETRY_TASK))
2706                         goto again;
2707
2708                 /* assumes fair_sched_class->next == idle_sched_class */
2709                 if (unlikely(!p))
2710                         p = idle_sched_class.pick_next_task(rq, prev);
2711
2712                 return p;
2713         }
2714
2715 again:
2716         for_each_class(class) {
2717                 p = class->pick_next_task(rq, prev);
2718                 if (p) {
2719                         if (unlikely(p == RETRY_TASK))
2720                                 goto again;
2721                         return p;
2722                 }
2723         }
2724
2725         BUG(); /* the idle class will always have a runnable task */
2726 }
</code></pre>
const struct sched_class fair_sched_class（kernel/sched/fair.c）

在CFS算法中，我们看下面有两个比较特殊：

<pre><code>	
7944 #ifdef CONFIG_SMP
7945 .select_task_rq = select_task_rq_fair,
7946 .migrate_task_rq = migrate_task_rq_fair,
</code></pre>

多CPU必然存在进程并行运行的情况，7945行是公平的选择特定的task，7956行是进行rq中task的迁移，我们知道每个cpu都对应着一个rq队列，这个不一定是quenu，而是red-black tree。对于rq中task的迁移，在

select_task_rq_fair(struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags)

###这个函数正是真正的完全公平调度算法! 

__schedule()函数是进程的主调度器，下面我们来分析这个的实现
<pre><code>	
2765 static void __sched __schedule(void)
2766 {
2767         struct task_struct *prev, *next;
2768         unsigned long *switch_count;
2769         struct rq *rq;
2770         int cpu;
2771
2772 need_resched:
2773         preempt_disable();
2774         cpu = smp_processor_id();
2775         rq = cpu_rq(cpu);
2776         rcu_note_context_switch(cpu);
2777         prev = rq->curr;
2778
2779         schedule_debug(prev);
2780
2781         if (sched_feat(HRTICK))
2782                 hrtick_clear(rq);
2783
2784         /*
2785          * Make sure that signal_pending_state()->signal_pending() below
2786          * can't be reordered with __set_current_state(TASK_INTERRUPTIBLE)
2787          * done by the caller to avoid the race with signal_wake_up().
2788          */
2789         smp_mb__before_spinlock();
2790         raw_spin_lock_irq(&rq->lock);
2791
2792         switch_count = &prev->nivcsw;
2793         if (prev->state && !(preempt_count() & PREEMPT_ACTIVE)) {
2794                 if (unlikely(signal_pending_state(prev->state, prev))) {
2795                         prev->state = TASK_RUNNING;
2796                 } else {
2797                         deactivate_task(rq, prev, DEQUEUE_SLEEP);
2798                         prev->on_rq = 0;
2799
2800                         /*
2801                          * If a worker went to sleep, notify and ask workqueue
2802                          * whether it wants to wake up a task to maintain
2803                          * concurrency.
2804                          */
2805                         if (prev->flags & PF_WQ_WORKER) {
2806                                 struct task_struct *to_wakeup;
2807
2808                                 to_wakeup = wq_worker_sleeping(prev, cpu);
2809                                 if (to_wakeup)
2810                                         try_to_wake_up_local(to_wakeup);
2811                         }
2812                 }
2813                 switch_count = &prev->nvcsw;
2814         }
2815
2816         if (task_on_rq_queued(prev) || rq->skip_clock_update < 0)
2817                 update_rq_clock(rq);
2818
2819         next = pick_next_task(rq, prev);
2820         clear_tsk_need_resched(prev);
2821         clear_preempt_need_resched();
2822         rq->skip_clock_update = 0;
2823
2824         if (likely(prev != next)) {
2825                 rq->nr_switches++;
2826                 rq->curr = next;
2827                 ++*switch_count;
2828
2829                 context_switch(rq, prev, next); /* unlocks the rq */
2830                 /*
2831                  * The context switch have flipped the stack from under us
2832                  * and restored the local variables which were saved when
2833                  * this task called schedule() in the past. prev == current
2834                  * is still correct, but it can be moved to another cpu/rq.
2835                  */
2836                 cpu = smp_processor_id();
2837                 rq = cpu_rq(cpu);
2838         } else
2839                 raw_spin_unlock_irq(&rq->lock);
2840
2841         post_schedule(rq);
2842
2843         sched_preempt_enable_no_resched();
2844         if (need_resched())
2845                 goto need_resched;
2846 }
</code></pre>

在2773 禁止进程抢占调度器，在2774 ~ 2777 获取当前cpu的id，并获取当前cpu的rq，切换RCU，获取当前rq运行的task，并赋值为prev。

<pre><code>
203 #define TASK_RUNNING            0
204 #define TASK_INTERRUPTIBLE      1
205 #define TASK_UNINTERRUPTIBLE    2
</code></pre>

我们发现TASK_RUNNING 值为0，这就使得2793行，如果判断当前的进程在运行，就不会进行调度，只会更新rq的clock。

反之如果当前占用cpu的task处于TASK_INTERRUPTIBLE态，却收到了某个唤醒它的信号，那么当前进程的标志被更新为TASK_RUNNING,等待再次被调度。否则，通过deactivate_task()将当前进程prev从就绪队列中删除。

之后在2819行使用pick_next_task()函数，去的当前rq的新的进程，然后清除之前prev进程的标志位。
获取要调度的新的进程，之后就是各种调度了。从2824~2839 这段代码会判断当前的选择的进程与之前的进程是否相同，相同就不用再切换上下文了。

一切调度完成，放开preempt_enable ，系统可以开始抢占。

参考：
[http://www.makelinux.net/books/lkd2/ch09lev1sec9](http://www.makelinux.net/books/lkd2/ch09lev1sec9)

