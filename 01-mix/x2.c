#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

int print(void);
static int __init xmod_init(void)
{
	print();
	return 0;
}

static void __exit xmod_exit(void)
{

}
module_init(xmod_init);
module_exit(xmod_exit);
MODULE_LICENSE("GPL");
