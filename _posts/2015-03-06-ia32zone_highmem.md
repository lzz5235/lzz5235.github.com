---
layout: post
title: "IA32下高端内存页框(ZONE_HIGHMEM)永久内核映射的分析与实现"
categories: linux
tags: Memory
---
IA32下高端内存页框(ZONE_HIGHMEM)永久内核映射的分析与实现
========================================================

###一、背景
在AMD64平台下，可用的线性地址空间远远大于目前安装的RAM大小，所以在这种体系中，ZONE_HIGHMEM总是空的。

而在32位平台上，Linux设计者必须找到一种方式允许内核使用所有4GB的RAM，如果IA32支持PAE的话，则使内核可以支持64GB的RAM。

IA32架构下，之所以有这些约束，主要是为了兼容前代固有的硬件架构(i386)，例如DMA直接读取，i386 内存大于900MB的情况。

ZONE_HIGHMEM属于高于896MB的内存区域，需要通过高端内存页框分配的方式映射到内核线性地址空间，从而使得内核可以对高端内存进行访问。而低于896MB直接通过（PAGE_OFFSET）偏移映射到物理内存上。详细查看[http://lzz5235.github.io/2015/01/04/32.html](http://lzz5235.github.io/2015/01/04/32.html)

###二、原理
高端内存页框分配alloc_pages()函数返回的是__page frame 页描述符（page description）的线性地址___，因为这些页描述符地址一旦被分配，便不再改变，所以他是__全局唯一的__。这些页描述符在free_area_init_nodes()初始化时，已建立好。

注：free_area_init_nodes()之前的初始化与体系结构相关，当系统调用这个函数后，开始初始化每个pg_data_t的结构体，便于体系结构再无相关，[http://lxr.free-electrons.com/source/mm/page_alloc.c#L5386](http://lxr.free-electrons.com/source/mm/page_alloc.c#L5386)。

##页描述符我们可以想象成系统将所有内存按照4k等分划分形成的数组下标。所有的page descriptor组成一个大的连续的数组 ，每个节点起始地址存放在 struct page *node_mem_map中，因此如果知道了page descriptor的地址pd，pd-node_mem_map就得到了pd是哪个page frame的描述符，也可以知道这个页描述符是否高于896MB。
 

已知alloc_pages()函数分配一个page descriptor的线性地址，可由它得到它所描述的物理页是整个内存的第几页：

假设是第N个物理页，那么这个物理页的物理地址是physAddr = N << PAGE_SHIFT   ，一般情况PAGE_SHIFT   为12 也就是4k。在得知该物理页的物理地址是physAddr后，就可以视physAddr的大小得到它的虚拟地址，然后对这个虚拟地址进行判断：

1.physAddr < 896M  对应虚拟地址是 physAddr + PAGE_OFFSET   (PAGE_OFFSET=3G)
2.physAddr >= 896M 对应虚拟地址不是静态映射的，通过内核的高端虚拟地址映射得到一个虚拟地址（也就是内核页表映射）。

在得到该页的虚拟地址之后，内核就可以正常访问这个物理页了。

###所以，我们可以将physAddr高于896M的这128M看做成为一个内核页表，这个内核页表在paging_init()初始化完成。

###三、实现
内核采取三种不同的机制将page frame映射到高端内存：永久内核映射、临时内核映射、非连续内存分配。

alloc_page()函数集返回的是全是struct page* 类型的数据，返回的也都是page descriptor的线性地址。然后系统要判断该page descriptor是否是highmem,也就是void *kmap(struct page *page)
[http://lxr.free-electrons.com/source/arch/x86/mm/highmem_32.c#L6](http://lxr.free-electrons.com/source/arch/x86/mm/highmem_32.c#L6) 。

 

系统调用PageHighMem(page)，判断当前page是否是highmem，如果是返回kmap_high(page)。如果不是返回page_address(page)  [http://lxr.free-electrons.com/source/mm/highmem.c#L412](http://lxr.free-electrons.com/source/mm/highmem.c#L412)

 

<pre><code>
void *kmap(struct page *page)
{
         might_sleep();
         if (!PageHighMem(page))
                 return page_address(page);
         return kmap_high(page);
}
void *page_address(const struct page *page)
{
         unsigned long flags;
         void *ret;
         struct page_address_slot *pas;
 
         if (!PageHighMem(page))
                 return lowmem_page_address(page);
 
         pas = page_slot(page);
         ret = NULL;
         spin_lock_irqsave(&pas->lock, flags);
         if (!list_empty(&pas->lh)) {
                 struct page_address_map *pam;
 
                 list_for_each_entry(pam, &pas->lh, list) {
                         if (pam->page == page) {
                                 ret = pam->virtual;
                                 goto done;
                         }
                 }
         }
 done:
         spin_unlock_irqrestore(&pas->lock, flags);
         return ret;
}
</code></pre>

在page_address（）函数中，我们要再次检查该page是否属于HIGHMEM，如果不属于，则直接计算偏移量__va(PFN_PHYS(page_to_pfn(page)))。

而pas = page_slot(page);之后的代码，是在永久映射中，系统为了方便查找，建立了一个page_address_htable散列表，系统可以很快的对于已经存放在散列表中的永久映射的page descriptor的线性地址进行查找，如果找到就返回内核线性地址，否则为NULL。

如果判断该page属于HIGHMEM，进入kmap_high(page) 
[http://lxr.free-electrons.com/source/mm/highmem.c#L279](http://lxr.free-electrons.com/source/mm/highmem.c#L279)

<pre><code>
void *kmap_high(struct page *page)
{
         unsigned long vaddr;
 
/*
* For highmem pages, we can't trust "virtual" until
* after we have the lock.
*/
         lock_kmap();
         vaddr = (unsigned long)page_address(page);
         if (!vaddr)
                 vaddr = map_new_virtual(page);
         pkmap_count[PKMAP_NR(vaddr)]++;
         BUG_ON(pkmap_count[PKMAP_NR(vaddr)] < 2);
         unlock_kmap();
         return (void*) vaddr;
}
</code></pre>

虽然判断了HIGHMEM，调用这个函数，但是我们不能信任，仍需要再次检查，顺便检查是否在page_address_htable散列表中存在该page，如果不存在，则进行映射（ map_new_virtual(page)），然后将pkmap_count数组中，这个page引用+1，然会内核就可以正常使用这个高端内存的线性地址了。

map_new_virtual()函数是进行HIGHMEM映射的核心函数，其中for(;;)循环是寻找可用的内核页表项进行高端内存页框映射，这里涉及到了页表color问题，主要为了提速查找效率。

当系统找到可用的页表项时，从line261~line268就是核心

* 以PKMAP_BASE为基址last_pkmap_nr为偏移在永久映射区创建内核线性地址vaddr

* 将vaddr加入pkmap_page_table

* 引用赋值为1，之后有可能会++

* 然后将该内核线性地址vaddr加入page，并返回内核线性地址

<pre><code>
217 static inline unsigned long map_new_virtual(struct page *page)
218 {
...
224 start:
225         count = get_pkmap_entries_count(color);
226         /* Find an empty entry */
227         for (;;) {
228                 last_pkmap_nr = get_next_pkmap_nr(color);
229                 if (no_more_pkmaps(last_pkmap_nr, color)) {
230                         flush_all_zero_pkmaps();
231                         count = get_pkmap_entries_count(color);
232                 }
233                 if (!pkmap_count[last_pkmap_nr])
234                         break;  /* Found a usable entry */
235                 if (--count)
236                         continue;
237
238                 /*
239                  * Sleep for somebody else to unmap their entries
240                  */
241                 {
242                         DECLARE_WAITQUEUE(wait, current);
243                         wait_queue_head_t *pkmap_map_wait =
244                                 get_pkmap_wait_queue_head(color);
246                         __set_current_state(TASK_UNINTERRUPTIBLE);
247                         add_wait_queue(pkmap_map_wait, &wait);
248                         unlock_kmap();
249                         schedule();
250                         remove_wait_queue(pkmap_map_wait, &wait);
251                         lock_kmap();
253                         /* Somebody else might have mapped it while we slept */
254                         if (page_address(page))
255                                 return (unsigned long)page_address(page);
256
257                         /* Re-start */
258                         goto start;
259                 }
260         }
261         vaddr = PKMAP_ADDR(last_pkmap_nr);
262         set_pte_at(&init_mm, vaddr,
263                    &(pkmap_page_table[last_pkmap_nr]), mk_pte(page, kmap_prot));
264
265         pkmap_count[last_pkmap_nr] = 1;
266         set_page_address(page, (void *)vaddr);
268         return vaddr;
</code></pre>

中间line241~259 内核考虑一种情况：如果这个PKMAP都已经被映射，最少的count都是>=1的，那么需要将当前的映射操作阻塞，然后加入等待队列，然后主动让出cpu被调度，直到PKMAP中存在count=0的页表项存在，唤醒，重新执行一次。

pkmap_count引用的可能值：0/1/>1。

0意味着该内核页表项未被映射高端内存，是可用的

1意味着虽然未被映射高端内存，但是需要unmapped，如果没有任何可用的页表项了，需要调用flush_all_zero_pkmaps()刷新pkmap_count，并置零。

大于1意味着该页表项被使用。

 

参考：

[http://repo.hackerzvoice.net/depot_madchat/ebooks/Mem_virtuelle/linux-mm/zonealloc.html#INITIALIZE](http://repo.hackerzvoice.net/depot_madchat/ebooks/Mem_virtuelle/linux-mm/zonealloc.html#INITIALIZE)

[http://blog.csdn.net/lcw_202/article/details/5955783](http://blog.csdn.net/lcw_202/article/details/5955783)

[https://www.kernel.org/doc/gorman/html/understand/understand006.html#sec: Mapping addresses to struct pages](https://www.kernel.org/doc/gorman/html/understand/understand006.html#sec: Mapping addresses to struct pages)
