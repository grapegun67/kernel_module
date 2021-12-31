#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>

//macro
#define ARRAY_INDEX	20
#define LINUX_KERNEL_VERSION	510
#define WR_VALUE	_IOW('a', 'a', int32_t *)
#define RD_VALUE	_IOR('a', 'b', int32_t *)

//global value
int32_t value = 0;
static int lenf = 1;
char test_array[ARRAY_INDEX] = "test_proc_array";

dev_t   dev = 0;
static struct class *dev_class;
static struct cdev test_cdev;
static struct proc_dir_entry *parent;

//fucntion
static int __init test_func_init(void);
static void __exit test_func_exit(void);

static int test_open(struct inode *inode, struct file *file);
static int test_release(struct inode *inode, struct file *file);
static ssize_t test_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t test_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static long test_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

//procfs function
static int test_proc_open(struct inode *inode, struct file *file);
static int test_proc_release(struct inode *inode, struct file *file);
static ssize_t test_proc_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t test_proc_write(struct file *filp, const char *buf, size_t len, loff_t *off);

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
        return (0);
}

static ssize_t test_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
        pr_info("WRITE DRIVER\n");
        return (len);
}

static long test_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
	case WR_VALUE:
		if (copy_from_user(&value, (int32_t *)arg, sizeof(value)) != 0)
		{
			pr_err("copy_from_user() error\n");
		}
		else
			pr_info("value: [%d]\n", value);
		break;

	case RD_VALUE:
		if (copy_to_user((int32_t *)arg, &value, sizeof(value)) != 0)
		{
			pr_err("copy_to_user() error\n");
		}
		else
			pr_info("succes copy_to_user!\n");
		break;

	}

	return (0);
}

//fops
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = test_read,
        .write          = test_write,
        .open           = test_open,
        .release        = test_release,
	.unlocked_ioctl	= test_ioctl,
};

//procfs fops
static int test_proc_open(struct inode *inode, struct file *file)
{
        pr_info("PROC OPEN DRIVER\n");
        return (0);
}

static int test_proc_release(struct inode *inode, struct file *file)
{
        pr_info("PROC RELEASE DRIVER\n");
        return (0);
}

static ssize_t test_proc_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("PROC READ DRIVER\n");
	if (lenf == 1)
		lenf = 0;
	else
	{
		lenf = 1;
		return (0);
	}

	if (copy_to_user(buf, test_array, ARRAY_INDEX) != 0)
		pr_err("test_proc_read -> copy_to_user() error\n");

        return (len);
}

static ssize_t test_proc_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
        pr_info("PROC WRITE DRIVER\n");
	
	if (copy_from_user(test_array, buf, len) != 0)
		pr_err("test_proc_write -> copy_from_user() error\n");

        return (len);
}

static struct proc_ops proc_fops =
{
        .proc_read           = test_proc_read,
        .proc_write          = test_proc_write,
        .proc_open           = test_proc_open,
        .proc_release        = test_proc_release,
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

	parent = proc_mkdir("test", NULL);
	if (parent == NULL)
	{
		pr_info("proc_mkdir() error\n");
		return (-1);
	}
	proc_create("test_proc", 0666, parent, &proc_fops);
	pr_info("proc success\n");

        pr_info("----success\n");

        return (0); 
}

static void __exit test_func_exit(void)
{
	proc_remove(parent);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Bye test module func!\n");
}

module_init(test_func_init);
module_exit(test_func_exit);
MODULE_LICENSE("GPL");
