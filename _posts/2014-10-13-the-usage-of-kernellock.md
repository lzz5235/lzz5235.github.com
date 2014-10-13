---
layout: post
title: "The Usage of KernelLock"
categories: linux
tags: kernel,spinlock,semaphore

---
内核锁的使用
=============
内核锁有三种形态：

* 原子操作
* spinlock
* semaphore

##1.原子结构

<pre><code>
typedef struct {
    int counter;
} atomic_t;
</code></pre>

不能直接赋值，需要使用函数比如ATOMIC_INIT()等初始化等函数(arch/alpha/include/asm/atomic.h)

初始化宏的源码很明显的说明了如何初始化一个原子变量。我们在定义一个原子变量时可以这样的使用它：

<pre><code>
atomic_t v=ATOMIC_INIT(0);
atomic_set函数可以原子的设置一个变量的值。
</code></pre>

###2.获取原子变量的值

<pre><code>
23static inline int atomic_read(const atomic_t *v)
24{
25        return (*(volatile int *)&(v)->counter);
26}
</code></pre>

返回原子型变量v中的counter值。关键字volatile保证&(v->counter)的值固定不变，以确保这个函数每次读入的都是原始地址中的值。

###3.原子变量的加与减

<pre><code>
47static inline void atomic_add(int i, atomic_t *v)
48{
49        asm volatile(LOCK_PREFIX "addl %1,%0"
50                     : "+m" (v->counter)
51                     : "ir" (i));
52}
 
61static inline void atomic_sub(int i, atomic_t *v)
62{
63        asm volatile(LOCK_PREFIX "subl %1,%0"
64                     : "+m" (v->counter)
65                     : "ir" (i));
66}
</code></pre>

加减操作中使用了内联汇编语句。linux中的汇编语句都采用AT&T指令格式。


##2.自旋锁spinlock（include/linux/spinlock_types.h)

<pre><code>
typedef struct spinlock {
    union {
        struct raw_spinlock rlock;
 
#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define LOCK_PADSIZE (offsetof(struct raw_spinlock, dep_map))
        struct {
            u8 __padding[LOCK_PADSIZE];
            struct lockdep_map dep_map;
        };
#endif
    };
} spinlock_t;
</code></pre>

自旋锁的使用：
###1.定义初始化自旋锁

使用下面的语句就可以先定一个自旋锁变量，再对其进行初始化：

<pre><code>
spinlock_t lock;
spin_lock_init(&lock);
</code></pre>

###2.获得自旋锁

<pre><code>
void spin_lock(spinlock_t*);
int spin_trylock(spinlock_t*);
</code></pre>

使用spin_lock(&lock)这样的语句来获得自旋锁。如果一个线程可以获得自旋锁，它将马上返回；否则，它将自旋至自旋锁的保持者释放锁。

另外可以使用spin_trylock(&lock)来试图获得自旋锁。如果一个线程可以立即获得自旋锁，则返回真；否则，返回假，此时它并不自旋。

###3.释放自旋锁

<pre><code>
void spin_unlock(spinlock_t*);
</code></pre>

使用spin_unlock(&lock)来释放一个已持有的自旋锁。注意这个函数必须和spin_lock或spin_trylock函数配套使用。

##3.信号量semaphore(include/linux/semaphore.h)

<pre><code>
struct semaphore {
    raw_spinlock_t      lock;
    unsigned int        count;
    struct list_head    wait_list;
};
 
#define __SEMAPHORE_INITIALIZER(name, n)                \
{                                   \
    .lock       = __RAW_SPIN_LOCK_UNLOCKED((name).lock),    \
    .count      = n,                        \
    .wait_list  = LIST_HEAD_INIT((name).wait_list),     \
}
 
#define DEFINE_SEMAPHORE(name)  \
    struct semaphore name = __SEMAPHORE_INITIALIZER(name, 1)
 
static inline void sema_init(struct semaphore *sem, int val)
{
    static struct lock_class_key __key;
    *sem = (struct semaphore) __SEMAPHORE_INITIALIZER(*sem, val);
    lockdep_init_map(&sem->lock.dep_map, "semaphore->lock", &__key, 0);
}
 
extern void down(struct semaphore *sem);
extern int __must_check down_interruptible(struct semaphore *sem);
extern int __must_check down_killable(struct semaphore *sem);
extern int __must_check down_trylock(struct semaphore *sem);
extern int __must_check down_timeout(struct semaphore *sem, long jiffies);
extern void up(struct semaphore *sem);
</code></pre>

信号量（semaphore）是保护临界区的一种常用方法。它的功能与自旋锁相同，只能在得到信号量的进程才能执行临界区的代码。但是和自旋锁不同的是，一个进程不能获得信号量时，它会进入睡眠状态而不是自旋。

semaphore的使用
1.定义初始化信号量

使用下面的代码可以定义并初始化信号量sem：

<pre><code>
struct semaphore sem;
sema_init(&sem,val);
</code></pre>

其中val即为信号量的初始值。

除上面的方法，可以使用下面的两个宏定义并初始化信号量：
<pre><code>
DECLARE_MUTEX(name);

DECLARE_MUTEX_LOCKED(name);
</code></pre>
其中name为变量名。

2.获得信号量

down(&sem);

进程使用该函数时，如果信号量值此时为0，则该进车会进入睡眠状态，因此该函数不能用于中断上下文中。

down_interruptibale(&sem);

该函数与down函数功能类似，只不过使用down而睡眠的进程不能被信号所打断，而使用down_interruptibale的函数则可以被信号打断。

如果想要在中断上下文使用信号量，则可以使用下面的函数：

dwon_try(&sem);

使用该函数时，如果进程可以获得信号量，则返回0；否则返回非0值，不会导致睡眠。

3.释放信号量

up(&sem);

该函数会释放信号量，如果此时等待队列中有进程，则唤醒一个。

三种比较常用的互斥方式

需求    建议加锁的方法

低开销加锁	优先使用自旋锁

短期加锁	优先使用自旋锁

长期加锁	优先使用信号量

中断上下文加锁	使用自旋锁

持有锁时需要睡眠	使用信号量

####自旋锁是忙等锁。当其他线程持有自旋锁时，如果另有线程想获得该锁，那么就只能循环的等待。这样忙的功能对CPU来说是极大的浪费。因此只有当由自旋锁保护的临界区执行时间很短时，使用自旋锁才比较合理。

[sharelist.c](/assets/resource/sharelist.c)

[spinlock.c](/assets/resource/spinlock.c)
