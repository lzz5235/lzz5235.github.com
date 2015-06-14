---
layout: post
title: "内核页表和进程页表"
categories: linux
tags: pagetable,kernel,userspace
---
内核页表和进程页表
===================
最近在看vmalloc()分配代码，我们知道当通过alloc_page()分配出来page后，需要将这些分散的物理页框page映射到vmalloc区，这里我们就要修改内核页表，以前我学页表是把内核空间与用户空间割裂学习的，导致二者无法很好地衔接，这里我会把两个概念重新解释清楚。

下面代码映射到vmalloc区的关键就是map_vm_area()函数，

<pre><code>
for (i = 0; i < area->nr_pages; i++) {
        struct page *page;
 
        if (node == NUMA_NO_NODE)
            page = alloc_page(alloc_mask);
        else
            page = alloc_pages_node(node, alloc_mask, order);
...
    }
 
    if (map_vm_area(area, prot, pages))
        goto fail;
    return area->addr;
</code></pre>
   
拿IA32架构的虚拟地址来说，0~3GB属于用户空间，用户空间是由每个进程的task_struct.mm_struct.pgd的成员变量，这个指向的就是进程页表。而3G~4GB属于内核空间，这个页表是由内核页目录表管理，存放在主内核页全局目录init_mm.pgd(swapper_pg_dir)中

<pre><code>
struct mm_struct init_mm = {
         .mm_rb          = RB_ROOT,
         .pgd            = swapper_pg_dir,
         .mm_users       = ATOMIC_INIT(2),
         .mm_count       = ATOMIC_INIT(1),
         .mmap_sem       = __RWSEM_INITIALIZER(init_mm.mmap_sem),
         .page_table_lock =  __SPIN_LOCK_UNLOCKED(init_mm.page_table_lock),
         .mmlist         = LIST_HEAD_INIT(init_mm.mmlist),
         INIT_MM_CONTEXT(init_mm)
};
</code></pre>

进程切换切换的是进程页表：即将新进程的pgd(页目录)加载到CR3寄存器中。而内核页表是所有进程共享的，每个进程的“进程页表”中内核态地址相关的页表项都是“内核页表”的一个拷贝。

###vmalloc区发生page fault

在vmalloc区发生page fault时，将“内核页表”同步到“进程页表”中。这部分区域对应的线性地址在内核使用vmalloc分配内存时，其实就已经分配了相应的物理内存，并做了相应的映射，建立了相应的页表项，但相关页表项仅写入了“内核页表”，并没有实时更新到“进程页表中”，内核在这里使用了“延迟更新”的策略，将“进程页表”真正更新推迟到第一次访问相关线性地址，发生page fault时，此时在page fault的处理流程中进行“进程页表”的更新。

所以linux中，只有进程的页表是时刻在更换的，而内核页表全局只有一份，所有进程共享内核页表！

