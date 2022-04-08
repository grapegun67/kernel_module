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

#define DEVICE_NAME     "ioctl_driver"

/* ioctl macro
1. 매직넘버(디바이스 구분 용도) 
2. 명령 번호
3. arg 데이터 크기
*/
#define READM   _IOR(0, 0, int32_t *)
#define WRITEM  _IOW(0, 1, int32_t *)

static int32_t value = 0;

/* fops function */
int cdev_open(struct inode* inode, struct file* file)
{
        printk(KERN_ALERT, "OPEN!\n");
        return (0);
}

int cdev_release(struct inode* inode, struct file* file)
{
        printk(KERN_ALERT, "CLOSE!\n");
        return (0);
}

static ssize_t cdev_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_ALERT, "READ!\n");
        return (0);
}

static ssize_t cdev_write(struct file *filep, const char *buf, size_t len, loff_t *off)
{
        printk(KERN_ALERT, "WRITE!\n");
        return (0);
}

long cdev_unlocked_ioctl(struct file* filep, unsigned int cmd, unsigned long arg)
{
        switch (cmd)
        {
        case READM:
                printk(KERN_ALERT "READM IOCTL!\n");
                if (copy_to_user((int32_t *)arg, &value ,sizeof(value)) != 0)
                        printk(KERN_ALERT "copy_to_user error!\n");
                break;
        case WRITEM:
                printk(KERN_ALERT "WRITEM IOCTL!\n");
                if (copy_from_user(&value, (int32_t *)arg, sizeof(value)) != 0)
                        printk(KERN_ALERT "copy_from_user error!\n");
                break;
        default:
                printk(KERN_ALERT "OTHER IOCTL!\n");
                break;
        }

        return (0);
}
/* Values       */
static dev_t dev;
static struct cdev cdev;
static struct class *dev_class;

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .open           = cdev_open,
        .release        = cdev_release,
        .read           = cdev_read,
        .write          = cdev_write,
        .unlocked_ioctl = cdev_unlocked_ioctl,
};

/* init, exit function */
int __init cdev_ioctl_init(void)
{
        /* allocate device number from kernel   */
        if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0)
        {
                printk(KERN_ALERT "alloc error!\n");
                return (-1);
        }

        printk(KERN_ALERT "Major: [%d], minor: [%d]\n", MAJOR(dev), MINOR(dev));

        cdev_init(&cdev, &fops);
        /* register cdev to kernel              */
        if (cdev_add(&cdev, dev, 1) < 0)
        {
                unregister_chrdev_region(dev, 1);
                return (-1);
        }

        if ((dev_class = class_create(THIS_MODULE, DEVICE_NAME)) == NULL)
        {
                unregister_chrdev_region(dev, 1);
                return (-1);
        }

        if ((device_create(dev_class, NULL, dev, NULL, DEVICE_NAME)) == NULL)
        {
                unregister_chrdev_region(dev, 1);
                class_destroy(dev_class);
                return (-1);
        }

        printk(KERN_ALERT "Init Finshed!\n");
}

void __exit cdev_ioctl_exit(void)
{
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "exit byebye!\n");
}

module_init(cdev_ioctl_init);
module_exit(cdev_ioctl_exit);

MODULE_AUTHOR("HEEGUN");
MODULE_LICENSE("GPL");
