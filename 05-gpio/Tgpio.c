#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/timer.h>


#define DEFAULT_TIMER_VAL_MS (500)

struct private_data {
	unsigned int gpio_num;
	bool toggle;
	struct timer_list timeout;
	atomic_t timer_val;
};

static ssize_t timer_timeout_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct private_data *priv = dev_get_drvdata(dev);
	int val;
	sscanf(buf, "%d", &val);
	atomic_set(&priv->timer_val, val);
	return count;
}

static DEVICE_ATTR_WO(timer_timeout);

static struct private_data private_data = {
	.gpio_num = 4,
	.toggle = false
};

static struct device device = {
};


void timer_handler(unsigned long data)
{
	struct device *device = (struct device *)data;
	struct private_data *priv = dev_get_drvdata(device);	

	gpio_set_value(priv->gpio_num, priv->toggle);

	priv->toggle = !priv->toggle;
	mod_timer(&priv->timeout, jiffies + msecs_to_jiffies(atomic_read(&priv->timer_val)));
}

static int __init tgpio_init(void)
{
	int err;

	device_initialize(&device);
	device.init_name = "TGPIO";

	err = device_register(&device);
	if (err) {
		printk(KERN_ERR "cannot register device\n");
		return -1;
	}

	err = devm_gpio_request_one(&device, private_data.gpio_num, GPIOF_DIR_OUT | GPIOF_INIT_LOW | GPIOF_EXPORT, "TGPIO-pin");
	if (err) {
		printk(KERN_ERR "cannot request gpio\n");
		goto gpio;
	}
	
	private_data.timeout.function = timer_handler;
	private_data.timeout.data = (unsigned long)&device;
	atomic_set(&private_data.timer_val, DEFAULT_TIMER_VAL_MS);
	dev_set_drvdata(&device, &private_data);
	
	dev_attr_timer_timeout.attr.mode = S_IWUSR | S_IWGRP;
	device_create_file(&device, &dev_attr_timer_timeout);

	init_timer(&private_data.timeout);
	mod_timer(&private_data.timeout, jiffies + msecs_to_jiffies(500));
	
	return 0;
gpio:
	device_unregister(&device);
	return -1;
}

static void __exit tgpio_exit(void)
{
	struct private_data *p = dev_get_drvdata(&device);
	devm_gpio_free(&device, p->gpio_num);
	del_timer(&p->timeout);
	device_remove_file(&device, &dev_attr_timer_timeout);
	device_unregister(&device);
}
module_init(tgpio_init);
module_exit(tgpio_exit);
MODULE_LICENSE("GPL");
