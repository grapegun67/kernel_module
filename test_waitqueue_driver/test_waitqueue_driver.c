#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/wait.h>

//global value
int wait_queue_flag;
uint32_t read_count;
static struct task_struct *wait_thread;
wait_queue_head_t wait_queue_test;

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
	wait_queue_flag = 1;
	wake_up_interruptible(&wait_queue_test);
        return (0);
}

static ssize_t test_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
        pr_info("WRITE DRIVER\n");
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

//waitqueue function
static int wait_function(void *unused)
{
	while (1)
	{
		pr_info("Waiting Event....\n");
		wait_event_interruptible(wait_queue_test, wait_queue_flag != 0);
		if (wait_queue_flag == 2)
		{
			pr_info("RECEIVED EXIT CODE 2\n");
			return (0);
		}

		pr_info("Event count:[%d]\n", ++read_count);
		wait_queue_flag = 0;
	}

	return (0);
}

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

	//waitqueue
	init_waitqueue_head(&wait_queue_test);

	wait_thread = kthread_create(wait_function, NULL, "WaitThread");
	if (wait_thread)
	{
		pr_info("Thread create successfully\n");
		wake_up_process(wait_thread);
	}

	else
		pr_info("Thread creation failed\n");

        pr_info("----success\n");

        return (0);
}

static void __exit test_func_exit(void)
{
	wait_queue_flag = 2;
	wake_up_interruptible(&wait_queue_test);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Bye test module func!\n");
}

module_init(test_func_init);
module_exit(test_func_exit);
MODULE_LICENSE("GPL");
