#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

#define DEFAULT_TIMER_VAL_MS (2000)

struct private_data {
	struct workqueue_struct *workqueue;
	struct work_struct work;
	struct timer_list timeout;
	atomic_t timer_val;
};

static void myrelease(struct device *dev)
{
	return;
}

static struct device device;

static ssize_t timer_timeout_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct private_data *priv = dev_get_drvdata(dev);
	int val;
	sscanf(buf, "%d", &val);
	atomic_set(&priv->timer_val, val);
	return count;
}

static DEVICE_ATTR_WO(timer_timeout);

static void work_func(struct work_struct *work)
{
	struct X {
		struct list_head head;
		volatile unsigned long data;
	};
	LIST_HEAD(mylist);
	int i;
	const int const_count = 1000000;
	struct X *head, *head_tmp;
	for (i = 0; i < const_count; i++) {
		struct X *x;
		x  = kmalloc(sizeof(struct X), GFP_KERNEL);
		x->data = i;
		list_add_tail(&x->head, &mylist);
	}
	
	list_for_each_entry_safe(head, head_tmp, &mylist, head) {
		unsigned long val;
		val = head->data;
		list_del(&head->head);
		kfree(head);
	}

	printk(KERN_INFO "work func\n");
}

static void timer_handler(unsigned long data)
{
	struct device *device = (struct device *)data;
	struct private_data *priv = dev_get_drvdata(device);	

	// schedule some work	
	//schedule_work(&priv->work);
	//queue_work(system_wq, &priv->work);
	queue_work(priv->workqueue, &priv->work);
	
	mod_timer(&priv->timeout, jiffies + msecs_to_jiffies(atomic_read(&priv->timer_val)));
}

static int __init probe(void)
{
	int err;
	struct private_data *private_data;

	device_initialize(&device);
	device.init_name = "TWorkqueue";
	device.release = myrelease;

	err = device_register(&device);
	if (err) {
		printk(KERN_ERR "cannot register device\n");
		return -1;
	}

	private_data = kzalloc(sizeof(struct private_data), GFP_KERNEL);
	private_data->timeout.function = timer_handler;
	private_data->timeout.data = (unsigned long)&device;
	atomic_set(&private_data->timer_val, DEFAULT_TIMER_VAL_MS);
	dev_set_drvdata(&device, private_data);
	
	dev_attr_timer_timeout.attr.mode = S_IWUSR | S_IWGRP;
	device_create_file(&device, &dev_attr_timer_timeout);

	init_timer(&private_data->timeout);
	mod_timer(&private_data->timeout, jiffies + msecs_to_jiffies(500));
	
	private_data->workqueue = create_workqueue("Tworkqueue");
	INIT_WORK(&private_data->work, work_func);
	
	return 0;
}

static void __exit remove(void)
{
	struct private_data *p = dev_get_drvdata(&device);
	cancel_work_sync(&p->work);
	del_timer(&p->timeout);
	device_remove_file(&device, &dev_attr_timer_timeout);
	device_unregister(&device);
	drain_workqueue(p->workqueue);
	destroy_workqueue(p->workqueue);
	kfree(p);
}
module_init(probe);
module_exit(remove);
MODULE_LICENSE("GPL");
