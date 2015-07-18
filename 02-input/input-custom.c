#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/timer.h>

struct custom_data {
	int dummy;
	struct timer_list timeout;
};

#define MYKEY KEY_1

static int i = 1;
static void timer_handler(unsigned long data)
{
	struct input_dev *input_dev = (struct input_dev *)data;
	struct custom_data *custom_data = input_get_drvdata(input_dev);
	
	input_report_key(input_dev, MYKEY, i);
	input_sync(input_dev);
	i = !i;
	mod_timer(&custom_data->timeout, jiffies + msecs_to_jiffies(100));
}

static int input_open(struct input_dev *dev)
{
	return 0;
}

static void input_close(struct input_dev *dev)
{

}
/*
static void event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{

}

static int connect(struct input_handler *handle, struct input_dev *dev, const struct input_device_id *id)
{
	return 0;
}

static void disconnect(struct input_handle *handle)
{

}

static void start(struct input_handle *handle)
{

}

static struct input_handler input_handler = {
	.event = event,
	.connect = connect,
	.disconnect = disconnect,
	.start = start,	
};
*/
static struct input_dev *input_dev;
static struct input_id input_id = {
	.bustype = BUS_XTKBD,
	.vendor = 0xb,
	.product = 0xc,
	.version = 0xd,
};

static int __init probe(void)
{
	struct custom_data *custom_data;

	printk(KERN_INFO "loading input device\n");

	input_dev = input_allocate_device();	
	input_dev->name = "HELLO";
	//set_bit(EV_KEY, input_dev->evbit);
	//set_bit(KEY_1, input_dev->keybit);
	input_dev->open = input_open;
	input_dev->close = input_close;
	input_dev->id = input_id;
	input_dev->phys = "example/inputX";
	input_set_capability(input_dev, EV_KEY, MYKEY);	

	custom_data = kzalloc(sizeof(*custom_data), GFP_KERNEL);
	input_set_drvdata(input_dev, custom_data);

	custom_data->timeout.function = timer_handler;
	custom_data->timeout.data = (unsigned long)input_dev;
	init_timer(&custom_data->timeout);

	mod_timer(&custom_data->timeout, jiffies + msecs_to_jiffies(5000));
	input_register_device(input_dev);
	return 0;
}

static void __exit remove(void)
{
	struct custom_data *data = input_get_drvdata(input_dev);
	del_timer(&data->timeout);
	input_unregister_device(input_dev);
	input_free_device(input_dev);
	kfree(data);
}

module_init(probe);
module_exit(remove);
