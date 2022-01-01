#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#define GPIO_21	 21

//global value
dev_t   dev = 0;
static struct class *dev_class;
static struct cdev test_cdev;

//fucntion
static int __init test_func_init(void);
static void __exit test_func_exit(void);

static int test_open(struct inode *inode, struct file *file);
static int test_release(struct inode *inode, struct file *file);
static ssize_t test_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t test_write(struct file *filp, const char *buf, size_t len, loff_t *off);

static int test_open(struct inode *inode, struct file *file)
{
        pr_info("OPEN DRIVER\n");
        return (0);
}

static int test_release(struct inode *inode, struct file *file)
{
        pr_info("RELEASE DRIVER\n");
        return (0);
}

static ssize_t test_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("READ DRIVER\n");

	uint8_t	gpio_state = 0;

	gpio_state = gpio_get_value(GPIO_21);

	len = 1;
	if (copy_to_user(buf, &gpio_state, len) > 0)
		pr_err("test_read -> copy_to_user() error\n");
	else
		pr_info("GPIO_21 data: [%d]\n", gpio_state);

        return (0);
}

static ssize_t test_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
        pr_info("WRITE DRIVER\n");

	uint8_t	rec_buf[10] = {0,};

	if (copy_from_user(rec_buf, buf, len) > 0)
		pr_err("test_write -> copy_from_user() error\n");
	else
		pr_info("GPIO_21 set data: [%c]\n", rec_buf[0]);

	if (rec_buf[0] == '1')
		gpio_set_value(GPIO_21, 1);
	else if (rec_buf[0] == '0')
		gpio_set_value(GPIO_21, 0);
	else
		pr_err("NO 1 or 0 data\n");

        return (len);
}

//fops
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = test_read,
        .write          = test_write,
        .open           = test_open,
        .release        = test_release,
};

//source
static int __init test_func_init(void)
{
        if ((alloc_chrdev_region(&dev, 0, 1, "test_Dev")) < 0)
        {
                pr_err("alloc_chrdev_region() error\n");
                return (-1);
        }

        pr_info("----Major: [%d], minor: [%d]\n", MAJOR(dev), MINOR(dev));

        // cdev source
        cdev_init(&test_cdev, &fops);
        if ((cdev_add(&test_cdev, dev, 1)) < 0)
        {
                pr_err("cdev_add() error\n");
                return (-1);
        }

        if ((dev_class = class_create(THIS_MODULE, "test_Dev")) == NULL)
        {
                pr_err("class_create() error\n");
                return (-1);
        }

        if ((device_create(dev_class, NULL, dev, NULL, "test_Dev")) == NULL)
        {
                pr_err("device_create() error\n");
                return (-1);
        }

	//gpio source
	if (gpio_is_valid(GPIO_21) == false)
	{
		pr_err("GPIO_%d is not valid\n", GPIO_21);
		return (-1);
	}

	if (gpio_request(GPIO_21, "TEST_GPIO_21") < 0)
	{
		pr_err("gpio_request error\n");
		return (-1);
	}

	gpio_direction_output(GPIO_21, 0);

	gpio_export(GPIO_21, false);

        pr_info("----success\n");

        return (0);
}

static void __exit test_func_exit(void)
{
	gpio_unexport(GPIO_21);
	gpio_free(GPIO_21);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Bye test module func!\n");
}

module_init(test_func_init);
module_exit(test_func_exit);
MODULE_LICENSE("GPL");
