#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/leds.h>
#include <linux/slab.h>

#define DEFAULT_TIMER_VAL_MS (500)

struct private_data {
	unsigned int gpio_num;
	bool toggle;
	struct timer_list timeout;
	atomic_t timer_val;
	enum led_brightness brightness;
	struct led_classdev *led_cdev;
};

static struct private_data private_data = {
	.gpio_num = 4,
	.toggle = false
};

static void myrelease(struct device *dev)
{
	return;
}
static struct device device = {
	.release = myrelease
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



void timer_handler(unsigned long data)
{
	struct device *device = (struct device *)data;
	struct private_data *priv = dev_get_drvdata(device);	
	if (priv->brightness == LED_OFF) {
		gpio_set_value(priv->gpio_num, 0);
		return;
	}
	gpio_set_value(priv->gpio_num, priv->toggle);

	priv->toggle = !priv->toggle;
	mod_timer(&priv->timeout, jiffies + msecs_to_jiffies(atomic_read(&priv->timer_val)));
}

static void led_cdev_brightness_set(struct led_classdev *led_cdev, enum led_brightness brightness)
{
	struct private_data *priv = dev_get_drvdata(led_cdev->dev);
	priv->brightness = brightness;	
	timer_handler((unsigned long)led_cdev->dev);
}

static enum led_brightness led_cdev_brightness_get(struct led_classdev *led_cdev)
{
	struct private_data *priv = dev_get_drvdata(led_cdev->dev);
	return priv->brightness;
}

static int led_cdev_blink_set(	struct led_classdev *led_cdev, 
				unsigned long *delay_on, 
				unsigned long *delay_off)
{
	return 0;	
}

static int __init probe(void)
{
	int err;
	struct led_classdev *led_cdev;	

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
	
	led_cdev = (struct led_classdev *) kzalloc(sizeof(*led_cdev), GFP_KERNEL);
	if (!led_cdev) {
		printk(KERN_ERR "cannot allocate led class device\n");
		goto led_cdev;
	};
	private_data.led_cdev = led_cdev;
	led_cdev->name = "led-my";
	led_cdev->brightness_set = led_cdev_brightness_set;
	led_cdev->brightness_get = led_cdev_brightness_get;
	led_cdev->blink_set = led_cdev_blink_set;
	err = led_classdev_register(&device, led_cdev);
	if (err) {
		printk(KERN_ERR "cannot register led class device\n");
		goto led_register;
	}
	return 0;
	
led_register:
	kfree(led_cdev);
led_cdev:
	devm_gpio_free(&device, private_data.gpio_num);
	del_timer(&private_data.timeout);
	device_remove_file(&device, &dev_attr_timer_timeout);
gpio:
	device_unregister(&device);
	return -1;
}

static void __exit remove(void)
{
	struct private_data *p = dev_get_drvdata(&device);
	led_classdev_unregister(p->led_cdev);
	kfree(p->led_cdev);
	devm_gpio_free(&device, p->gpio_num);
	del_timer(&p->timeout);
	device_remove_file(&device, &dev_attr_timer_timeout);
	device_unregister(&device);
}
module_init(probe);
module_exit(remove);
MODULE_LICENSE("GPL");
