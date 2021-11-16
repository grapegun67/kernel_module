#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

int hello_init(void)
{
        printk(KERN_ALERT "Hello world\n");
}

void hello_exit(void)
{
        printk(KERN_ALERT "Good by world\n");
}

module_init(hello_init);
module_exit(hello_exit);
