---
layout: post
title: "machine_check_poll() 函数分析"
categories: linux
tags: MCA, machine_check_poll
---
machine_check_poll() 函数分析
==========================
machine_check_poll() 函数在kernel-3.18前，这个函数基本只是起一个记录的作用（调用mce_log()），在kernel-3.18后，加入的patch，可以使得该函数处理UCNA类型的错误。其他部差异不大，仅仅是扫描记录而已。而且在mce-severity.c中特定写入了UCNA类型的错误表。这个错误正是Intel manual中所规定的。

<pre><code>
enum severity_level {
         MCE_NO_SEVERITY,
         MCE_DEFERRED_SEVERITY,
         MCE_UCNA_SEVERITY = MCE_DEFERRED_SEVERITY,
         MCE_KEEP_SEVERITY,
         MCE_SOME_SEVERITY,
         MCE_AO_SEVERITY,
         MCE_UC_SEVERITY,
         MCE_AR_SEVERITY,
         MCE_PANIC_SEVERITY,
};
</code></pre>

这里为了兼容之前的MCE_KEEP_SEVERITY，加入了MCE_DEFERRED_SEVERITY 和 MCE_UCNA_SEVERITY 两个标志，并在machine_check_poll()中使用。

这里我们要注意的一点是：machine_check_poll工作在中断上下文，在调用mce_schedule_work()后，实际是通过调用工作队列中的mce_process_work() ，而这个函数会读取ring buffer中的错误数据。

<pre><code>
@@ -630,6 +662,20 @@ void machine_check_poll(enum mcp_flags flags, mce_banks_t *b)
 
                if (!(flags & MCP_TIMESTAMP))
                        m.tsc = 0;
+
+               severity = mce_severity(&m, mca_cfg.tolerant, NULL, false);
+
+               /*
+                * In the cases where we don't have a valid address after all,
+                * do not add it into the ring buffer.
+                */
+               if (severity == MCE_DEFERRED_SEVERITY && memory_error(&m)) {
+                       if (m.status & MCI_STATUS_ADDRV) {
+                               mce_ring_add(m.addr >> PAGE_SHIFT);
+                               mce_schedule_work();
+                       }
+               }
+
</code></pre>

这个时候我们看到memory_failure()已经工作在进程上下文，如果我们要对错误进行过滤的话，可以考虑在memory_failure()函数开头设置hook函数。

<pre><code>
static void mce_process_work(struct work_struct *dummy)
{
               unsigned long pfn;
               while (mce_ring_get(&pfn))
                      memory_failure(pfn , MCE_VECTOR , 0);
}
</code></pre>

参考：

[1] Intel Manual

