#include <linux/init.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/slab.h>

#define NTHREADS 200

struct my_struct
{
	struct list_head list;
	int id;
	int pid;
};

static struct work_struct queue;
static struct timer_list mytimer;
static LIST_HEAD(mine);
static unsigned int list_len = 0;
static DEFINE_SEMAPHORE(sem);
static DEFINE_SPINLOCK(my_lock);
static atomic_t my_count = ATOMIC_INIT(0);
static long count = 0;
//static int timer_over=0;


static int sharelist(void *data);
static void kthread_launcher(struct work_struct *q);
static void start_kthread(void);
void qt_task(unsigned long data);

static int __init share_init(void)
{
	int i;
	printk(KERN_INFO"share list enter\n");

	INIT_WORK(&queue,kthread_launcher);
	setup_timer(&mytimer,qt_task,0);
	add_timer(&mytimer);

	for(i=0;i<NTHREADS;i++)
	{
		start_kthread();
	}
	return 0;
}

static void __exit share_exit(void)
{
	struct list_head *n,*p=NULL;
	struct my_struct *my=NULL;
	printk("\nshare list exit\n");
	del_timer(&mytimer);
	spin_lock(&my_lock);

	list_for_each_safe(p,n,&mine);
	if(count++%4==0)
	{
		my=list_entry(p,struct my_struct,list);
		list_del(p);
		if(my!=NULL)
		{
	//		kill_proc(my->pid,SIGKILL,1);
			printk("SYSCALL DEL:%d\t",my->id);
		}
			kfree(my);
	}
	spin_unlock(&my_lock);
	printk(KERN_INFO"Over\n");
}

static int sharelist(void *data)
{
	struct my_struct *p;
	if(count++%4 ==0)
		printk("\n");
	spin_lock(&my_lock);
	
	if(list_len<100)
	{
		if((p=kmalloc(sizeof(struct my_struct),GFP_KERNEL))==NULL)
			return -ENOMEM;
		p->id = atomic_read(&my_count);
		atomic_inc(&my_count);
		p->pid = current->pid;
		list_add(&p->list,&mine);
		list_len++;
		printk("THREAD ADD:%-5d\t",p->id);
	}
	else
	{
		struct my_struct *my=NULL;
		my=list_entry(mine.prev,struct my_struct,list);
		list_del(mine.prev);
		list_len--;
		printk("THREAD DEL:%-5d\t",my->id);
		kfree(my);
	}
	spin_unlock(&my_lock);
	return 0;
}

void kthread_launcher(struct work_struct *q)
{
	kthread_run(sharelist,NULL,"sharelist");
	up(&sem);
}

static void start_kthread(void)
{
	down(&sem);
	schedule_work(&queue);
}

void qt_task(unsigned long data)
{
	if(!list_empty(&mine))
	{
		struct my_struct *i;
		if(count++%4==0)
		{
			printk("\n");
		}
		i = list_entry(mine.next,struct my_struct,list);
		list_del(mine.next);
		list_len--;
		printk("TIMER DEL:%-5d\t",i->id);
		kfree(i);
	}
	mod_timer(&mytimer,jiffies+1);
}



module_init(share_init);
module_exit(share_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lzz");
MODULE_DESCRIPTION("consumer and producer");
MODULE_ALIAS("simple module");
