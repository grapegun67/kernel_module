#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

//global value
dev_t   dev;
int	test_value;
static struct class *dev_class;
static struct cdev test_cdev;
struct kobject *kobj_ref;

//fucntion
static int __init test_func_init(void);
static void __exit test_func_exit(void);

static int test_open(struct inode *inode, struct file *file);
static int test_release(struct inode *inode, struct file *file);
static ssize_t test_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t test_write(struct file *filp, const char *buf, size_t len, loff_t *off);

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

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

//fops
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = test_read,
        .write          = test_write,
        .open           = test_open,
        .release        = test_release,
};

//sysfs
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	pr_info("sysfs SHOW!!\n");
	return (sprintf(buf, "%d", test_value));
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	pr_info("sysfs STORE!!\n");
	sscanf(buf, "%d", &test_value);
	return (count);
}

struct kobj_attribute test_attr = __ATTR(test_value, 0660, sysfs_show, sysfs_store);

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

	kobj_ref = kobject_create_and_add("test_sysfs", kernel_kobj);
	if (sysfs_create_file(kobj_ref, &test_attr.attr) != 0)
	{
		pr_err("sysfs_create_file() error\n");
		return (-1);
	}

        pr_info("----success\n");

        return (0);
}

static void __exit test_func_exit(void)
{
	kobject_put(kobj_ref);
	sysfs_remove_file(kernel_kobj, &test_attr.attr);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Bye test module func!\n");
}

module_init(test_func_init);
module_exit(test_func_exit);
MODULE_LICENSE("GPL");
