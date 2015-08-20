---
layout: post
title: "do_machine_check() 函数分析"
categories: linux
tags: linux,MCA
---
do_machine_check() 函数分析
========================
do_machine_check()函数是MCA架构最为核心的函数，在之前的 一篇博文 我们分析了现有MCA 对于现有错误的处理流程，但是没有对于do_machine_check函数进行分析。这里我们会深入分析这个异常处理函数，下面分析的代码是基于Linux-3.14.38，在今天一月份Andi Kleen 提交了最新的补丁，这个会在最后进行说明。

* do_machine_check()异常处理函数是由18号异常触发，它执行在NMI上下文，不适用已有的kernel service服务和机制，甚至无法正常打印信息。开头主要是声明一些变量，mca_config 主要用来声明当前系统mca的基本配置，包括是否选择通过mcelog记录CE错误，CMCI中断使能等。

<pre><code>
struct mca_config {
        bool dont_log_ce;
        bool cmci_disabled;
        bool ignore_ce;
        bool disabled;
        bool ser;
        bool bios_cmci_threshold;
        u8 banks;
        s8 bootlog;
        int tolerant;
        int monarch_timeout;
        int panic_timeout;
        u32 rip_msr;
};
</code></pre>

severity 主要记录错误等级，order主要用于后来进入Monarch’s region的顺序，no_way_out 用来快速检查当前cpu是否需要panic，mce_gather_info(&m, regs);用来收集当前cpu MCA寄存器中的数据，保存到struct mce结构体中。

mces_seen是每个cpu都有存在的struct mce变量，当需要保存该值时，就将其保存在final中，final<=>mces_seen等价。

<pre><code>
void do_machine_check(struct pt_regs *regs, long error_code)
{
        struct mca_config *cfg = &mca_cfg;
        struct mce m, *final;
        int i;
        int worst = 0;
        int severity;
 
        int order;
        int no_way_out = 0; 
 
        int kill_it = 0;
        DECLARE_BITMAP(toclear, MAX_NR_BANKS);
        DECLARE_BITMAP(valid_banks, MAX_NR_BANKS);
        char *msg = "Unknown";
        atomic_inc(&mce_entry);
 
        this_cpu_inc(mce_exception_count);
 
        if (!cfg->banks)
                goto out;
 
        mce_gather_info(&m, regs);
 
        final = &__get_cpu_var(mces_seen);
        *final = m;
 
        memset(valid_banks, 0, sizeof(valid_banks));
        no_way_out = mce_no_way_out(&m, &msg, valid_banks, regs);
 
        barrier();
</code></pre>

检查 mcgstatus 寄存器中的MCG_STATUS_RIPV是否有效，无效，将kill_it置1，后续配合SRAR事件使用。

mce_start()与mce_end()可以使得所有cpu进入Monarch’s region，第一个进入handler的是Monarch，之后的cpu听从Monarch的指挥。之后按照cfg配置信息，扫描当前cpu所有bank，将当前MSR_IA32_MCx_STATUS()信息读取到m.status中，判断这个status是否有效，对于machine_check_poll() 处理的事件跳过处理。

判断当前bank内严重等级，如果severity错误等级是MCE_KEEP_SEVERITY 和 MCE_NO_SEVERITY ，忽略这次扫描，然后mce_read_aux()继续读取ADDR与MISC相关信息到当前struct mce m中。

这时当前severity等级是MCE_AO_SEVERITY 时，kernel会将其保存到ring buffer中，在之后的work queue中进行处理。然后调用mce_log(&m)记录到/dev/mcelog中。如果此次错误等级最高，那么更新 worst ，并struct mce写入到当前cpu。

扫描完毕之后，保存struct mce 信息，清除寄存器内容。再次判断worst 等级，高于MCE_PANIC_SEVERITY的话，稍后会panic。然后mce_end()退出 Monarch。

<pre><code>
if (!(m.mcgstatus & MCG_STATUS_RIPV))
        kill_it = 1;
 
order = mce_start(&no_way_out);
for (i = 0; i < cfg->banks; i++) {
        __clear_bit(i, toclear);
        if (!test_bit(i, valid_banks))
                continue;
        if (!mce_banks[i].ctl)
                continue;
 
        m.misc = 0;
        m.addr = 0;
        m.bank = i;
 
        m.status = mce_rdmsrl(MSR_IA32_MCx_STATUS(i));
        if ((m.status & MCI_STATUS_VAL) == 0)
                continue;
 
        if (!(m.status & (cfg->ser ? MCI_STATUS_S : MCI_STATUS_UC)) &&
                !no_way_out)
                continue;
 
        add_taint(TAINT_MACHINE_CHECK, LOCKDEP_NOW_UNRELIABLE);
 
        severity = mce_severity(&m, cfg->tolerant, NULL);
 
        if (severity == MCE_KEEP_SEVERITY && !no_way_out)
                continue;
        __set_bit(i, toclear);
        if (severity == MCE_NO_SEVERITY) {
                continue;
        }
 
        mce_read_aux(&m, i);
 
        if (severity == MCE_AO_SEVERITY && mce_usable_address(&m))
                mce_ring_add(m.addr >> PAGE_SHIFT);
 
        mce_log(&m);
 
        if (severity > worst) {
                *final = m;
                worst = severity;
        }
}
m = *final;
 
if (!no_way_out)
        mce_clear_state(toclear);
 
if (mce_end(order) less  0)
        no_way_out = worst >= MCE_PANIC_SEVERITY;
 </code></pre>
 
tolerant 在mca机制中可调， 0最严格，3最宽松。

> /*
* Tolerant levels:
* 0: always panic on uncorrected errors, log corrected errors
* 1: panic or SIGBUS on uncorrected errors, log corrected errors
* 2: SIGBUS or log uncorrected errors (if possible), log corr. errors
* 3: never panic or SIGBUS, log all errors (for testing only)
*/

只要小于3，如果no_way_out等于1 ，直接panic，如果不是，worst等级是SRAR，那么标记这个进程，在返回用户态时处理，具体处理的函数就是memory_failure()；如果不是SRAR错误，而且RIPV无效，那么只能杀死当前进程。

<pre><code>
        if (cfg->tolerant less 3) {
                if (no_way_out)
                        mce_panic("Fatal machine check on current CPU", &m, msg);
                if (worst == MCE_AR_SEVERITY) {
                        /* schedule action before return to userland */
                        mce_save_info(m.addr, m.mcgstatus & MCG_STATUS_RIPV);
                        set_thread_flag(TIF_MCE_NOTIFY);
                } else if (kill_it) {
                        force_sig(SIGBUS, current);
                }
        }
 
        if (worst > 0)
                mce_report_event(regs);
        mce_wrmsrl(MSR_IA32_MCG_STATUS, 0);
out:
        atomic_dec(&mce_entry);
        sync_core();
}
</code></pre>

在最新代码 Linux-4.0.4 中，Andi Kleen 删除了mce_info，mce_save_info(),mce_find_info(),mce_clear_info(),mce_notify_process()和位于do_notify_resume()中的mce_notify_process()，也就是说SRAR不在返回用户态前处理。

> x86, mce: Get rid of TIF_MCE_NOTIFY and associated mce tricks
We now switch to the kernel stack when a machine check interrupts
during user mode. This means that we can perform recovery actions
in the tail of do_machine_check()

他改变了SRAR发生在用户空间时，通过设置fiag并调度的方式，直接在do_machine_check()最后加入对于这种错误的处理，并在末位加入memory_failure()的错误恢复，这里指出如果恢复失败，那么直接使用force_sig(SIGBUS, current)。

最后指出:do_machine_check()只处理SRAR、SRAO类型的错误，对于UCNA类型错误由machine_check_poll()处理，下篇博文介绍machine_check_poll()。

 

###参考：

[http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html)
