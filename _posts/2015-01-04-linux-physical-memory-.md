---
layout: post
title: "Linux 中Physical Memory 数据结构分析"
categories: linux 
tags: Memory
---
Linux 中Physical Memory 数据结构分析
===============================
Kernel在在管理内存时将物理内存从逻辑上划分为节点（node），内存管理区（zone），页框（frame page）三级结构。我们都知道frame page是管理内存单元的最小单位，这个frame page在代码中就是struct page。

而node是与cpu数量相关的！默认在NUMA存在多个cpu，则每个cpu都存在一个struct pglist_data 类型的节点。而一个struct pglist_data下又把当前管理的内存区域划分为3部分：这个就是由zone定义的。

zone将内存区域划分为三种类型：1）DMA  2）NORMAL 3）HIGHEM

引入这种node管理方式的根本原因是为了兼容UMA架构的计算机，Kernel对于内存的管理主要存在NUMA/UMA两种形式

关于这两种形式的解释，请查阅wiki：

[http://en.wikipedia.org/wiki/Non-uniform_memory_access](http://en.wikipedia.org/wiki/Non-uniform_memory_access)

[http://en.wikipedia.org/wiki/Uniform_memory_access](http://en.wikipedia.org/wiki/Uniform_memory_access)

下面来说明这三种结构体：

##1.struct page （include/linux/mm_types.h）

page结构体描述一个物理页框，每个物理页框都关联一个这样的结构体。但是page结构仅用来描述一个页框的属性，它不包含该页框中的任何数据。我们知道一个物理页框大小通常是4096 byte （4KB） 而sizeof（struct page）远远小于4096byte，其他空间用于存储数据。

内核在定义page结构时使用了许多union，这样做的目的是保证page结构尽可能的小。虽然每个page结构体占很少内存，但是由于实际系统中页框总数量巨大，因此所有页框对应的page结构所占用的内存量也很庞大。

##2.struct zone（include/linux/mmzone.h）

[http://lxr.free-electrons.com/source/include/linux/mmzone.h](http://lxr.free-electrons.com/source/include/linux/mmzone.h)

内核将内存划分为几个连续的区域，每个区域页框都是连续的。kernel使用枚举zone_type方式定义每个内存区域。

<pre><code>
enum zone_type {
#ifdef CONFIG_ZONE_DMA
    ZONE_DMA,
#endif
#ifdef CONFIG_ZONE_DMA32
    ZONE_DMA32,
#endif
    ZONE_NORMAL,
#ifdef CONFIG_HIGHMEM
    ZONE_HIGHMEM,
#endif
    ZONE_MOVABLE,
    __MAX_NR_ZONES
};
</code></pre>

struct zone -> struct per_cpu_pageset -> struct per_cpu_pages 可以记录每个内存管理区中page使用情况。

 内存管理区是一个逻辑上的概念，它的存在是因为计算机中硬件访问物理内存时有一些限制。因此，每个内存管理区的实际分布是与体系结构相关的。

下面来解释这三个主要的区域:
>    ZONE_DMA：某些设备通过DMA方式访问内存时，不能访问到所有的物理内存，此时只能为它们单独划分一块内存管理区。ZONE_DMA的范围根据体系结构而改变。

>    ZONE_NORMAL：这个区域包含的都是能够正常映射的页框。通过源码中的定义可以发现，所有的体系架构都包含这个区域。但是并不是每个架构下该区都能对应到实际的物理内存，根据上面所述，某些架构下ZONE_DMA32会占据整个4G的物理内存，因此该区域为空。

>   ZONE_HIGHMEM：这个区域代表超出内核空间大小的物理内存，这部分内存也被成为高端内存（与之对应ZONE_DMA和ZONE_NORMAL成为低端内存）。在32位的x86系统中，高端内存即为大于896MB的物理内存。而在64位的系统中，高端内存总为空。

>    __MAX_NR_ZONES：它用来标记内存管理区的数量。在UMA架构下，该常量为1.

##3.struct pglist_data

下面我们来看这个struct pglist_data

节点这个概念是由于NUMA（非一致内存访问）模型而诞生的，该模型只存在于多处理器计算机中。NUMA根据CPU数量将整个物理内存分为几个大块，每块内存即为每个CPU的的本地内存。这样的划分使每个CPU都能以较快的速度访问本地内存，当然每个CPU也可以访问其他CPU的内存只不过速度比较慢而已。上述的每块物理内存对应一个pg_data_t数据结构，每块物理内存即为一个节点，所以的结点形成一个双链表。

![](/assets/pic/NUMA_Memory.png)

<pre><code>
struct bootmem_data;
typedef struct pglist_data {
        struct zone node_zones[MAX_NR_ZONES];
        struct zonelist node_zonelists[MAX_ZONELISTS];
        int nr_zones;
#ifdef CONFIG_FLAT_NODE_MEM_MAP /* means !SPARSEMEM */
        struct page *node_mem_map;
#ifdef CONFIG_CGROUP_MEM_RES_CTLR
        struct page_cgroup *node_page_cgroup;
#endif
#endif
#ifndef CONFIG_NO_BOOTMEM
        struct bootmem_data *bdata;
#endif
#ifdef CONFIG_MEMORY_HOTPLUG
        spinlock_t node_size_lock;
#endif
        unsigned long node_start_pfn;
        unsigned long node_present_pages; /* total number of physical pages */
        unsigned long node_spanned_pages; /* total size of physical page
                                             range, including holes */
        int node_id;
        wait_queue_head_t kswapd_wait;
        struct task_struct *kswapd;
        int kswapd_max_order;
} pg_data_t;
</code></pre>

这个结构体里面的 struct page *node_mem_map;是指向page实例数组指针，用于描述节点的所有物理内存页。他包含了节点这个node所有的page！

node_zones：当前节点中内存管理区描述符数组。这个数组的大小使用__MAX_NR_ZONES来定义。

node_zonelists：它是zonelist结构的数组，长度为MAX_ZONELISTS。如果内核未配置NUMA，则长度为1，否则，长度为2。该数组中0号元素指定了备用的内存管理区链表，也就是当前系统中所有的zone。1号元素指定了当前节点中的管理区链表。除非分配内存时指定了GFP_THISNODE标志而采用本地内存节点上的zonelist，一般均采用备用zonelist。

unsigned long node_start_pfn; 上面结构体里的成员变量是该pg_data_t结构体的第一个pfn！三种内存区域都可能是存在的！

![](/assets/pic/understand-html001.png)

上面这个图比较老，那个zone_mem_map结构已经不存在，而是由unsigned long zone_start_pfn 地址来代替，通过这种方式查找出来的pfn是某个内存区域特有的！比如是ZONE_DMA/ZONE_NORMAL/ZONE_HIGHMEM的第一个pfn号码！

总的来说，主要是通过struct pglist_data -> struct zone(node_zones 三个不同的区域)->struct zone 下的unsigned long zone_start_pfn 找到某个特定的page！pfn全kernel唯一！ 

 

[ttps://www.kernel.org/doc/gorman/html/understand/understand005.html](ttps://www.kernel.org/doc/gorman/html/understand/understand005.html)

[http://lxr.free-electrons.com/source/include/linux/mmzone.h](http://lxr.free-electrons.com/source/include/linux/mmzone.h)

