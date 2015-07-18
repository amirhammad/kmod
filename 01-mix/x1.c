#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

int print(void)
{
	printk(KERN_ERR "HELLO\n");
}

MODULE_LICENSE("GPL");
