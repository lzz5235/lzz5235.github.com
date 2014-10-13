#include <linux/init.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define PRODUCT_NUMs 10

DEFINE_SEMAPHORE(sem_producer);
DEFINE_SEMAPHORE(sem_consumer);

static char product[12];
static atomic_t num = ATOMIC_INIT(0);
static int producer(void *product);
static int consumer(void *product);
static int id=1;
static int consume_num =0;

static int producer(void *p)
{
	char *product=(char *)p;
	int i;
	printk("producer[%d] start...\n",current->pid);
	atomic_inc(&num);
	for(i=0;i<PRODUCT_NUMs;i++)
	{
		down(&sem_producer);
	//	atomic_inc(&num);
		snprintf(product,12,"2014-09-%d",id++);
		printk("producer[%d] produce %s\n",current->pid,product);
	//	atomic_dec(&num);
		up(&sem_consumer);
	}
	printk("producer[%d] exit...\n",current->pid);
	return 0;
}

static int consumer(void *p)
{
	char *product=(char *)p;

	printk("consumer[%d] start....\n",current->pid);

	while(1)
	{
		msleep(100);
		down_interruptible(&sem_consumer);
		if(consume_num >=PRODUCT_NUMs*atomic_read(&num))
			break;
	//	atomic_inc(&num);
		printk("consumer[%d] consume %s\n",current->pid,product);
		consume_num++;
		memset(product,'\0',12);
	//	atomic_dec(&num);
		up(&sem_producer);
	}
	printk("consumer[%d] exit....\n",current->pid);
	return 0;
}

static int procon_init(void)
{
	sema_init(&sem_producer,1);
	sema_init(&sem_consumer,0);

	atomic_set(&num,0);

	kthread_run(producer,product,"producer");
	kthread_run(consumer,product,"consumer");
	printk(KERN_INFO"show producer and consumer\n");
	return 0;
}

static void procon_exit(void)
{
	printk(KERN_INFO"exit producer and consumer\n");
}



module_init(procon_init);
module_exit(procon_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lzz");
MODULE_DESCRIPTION("consumer and producer");
MODULE_ALIAS("simple module");
