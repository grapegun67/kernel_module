#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define GPIO_HIGH       1
#define GPIO_LOW        0

#define ECHO_VALID      1
#define ECHO_UNVALID    0

#define HC_SR04_ECHO_PIN        23
#define HC_SR04_TRIG_PIN        24

static int gpio_irq = -1;
static int echo_valid_flag = 0;
static ktime_t echo_start;
static ktime_t echo_stop;

//global value
int     test_value;
struct kobject *kobj_ref;

//fucntion
static int __init test_sys_init(void);
static void __exit test_sys_exit(void);

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

//interrupt
static irqreturn_t echo_interrupt_handler(int irq, void* data)
{
        ktime_t time_value;

        if (echo_valid_flag == ECHO_UNVALID)
        {
                time_value = ktime_get();
                if (gpio_get_value(HC_SR04_ECHO_PIN) == GPIO_HIGH)
                        echo_start = time_value;
                else
                {
                        echo_stop = time_value;
                        echo_valid_flag = ECHO_VALID;
                }

        }

        return (IRQ_HANDLED);
}

//sysfs
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        pr_info("sysfs SHOW!!\n");

        gpio_set_value(HC_SR04_TRIG_PIN, GPIO_HIGH);
        udelay(10);
        gpio_set_value(HC_SR04_TRIG_PIN, GPIO_LOW);

        echo_valid_flag = ECHO_UNVALID;

        udelay(1);

        return (sprintf(buf, "%lld\n", ktime_to_us(ktime_sub(echo_stop, echo_start))));
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        pr_info("sysfs STORE!!\n");
        sscanf(buf, "%d", &test_value);
        return (count);
}

struct kobj_attribute test_attr = __ATTR(test_value, 0660, sysfs_show, sysfs_store);

//source
static int __init test_sys_init(void)
{
        int ret;

        kobj_ref = kobject_create_and_add("test_sysfs", kernel_kobj);
        if (sysfs_create_file(kobj_ref, &test_attr.attr) != 0)
        {
                kobject_put(kobj_ref);
                pr_err("sysfs_create_file() error\n");
                return (-1);
        }

        ret = gpio_request_one(HC_SR04_TRIG_PIN, GPIOF_DIR_OUT, "TRIG");
        if (ret < 0)
        {
                kobject_put(kobj_ref);
                sysfs_remove_file(kernel_kobj, &test_attr.attr);
                pr_err("gpio_request_one - TRIG ERROR\n");
                return (-1);
        }

        ret = gpio_request_one(HC_SR04_ECHO_PIN, GPIOF_DIR_IN, "ECHO");
        if (ret < 0)
        {
                kobject_put(kobj_ref);
                sysfs_remove_file(kernel_kobj, &test_attr.attr);
                pr_err("gpio_request_one - ECHO ERROR\n");
                return (-1);
        }

        ret = gpio_to_irq(HC_SR04_ECHO_PIN);
        if (ret < 0)
        {
                kobject_put(kobj_ref);
                sysfs_remove_file(kernel_kobj, &test_attr.attr);
                pr_err("gpio_request_one - ECHO ERROR\n");
                return (-1);
        }

        else
                gpio_irq = ret;

        ret = request_irq(gpio_irq, echo_interrupt_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "hc-sr04-echo", NULL);

        if (ret != 0)
                return (-1);

        pr_info("----success\n");

        return (0);
}

static void __exit test_sys_exit(void)
{
        kobject_put(kobj_ref);
        sysfs_remove_file(kernel_kobj, &test_attr.attr);
        gpio_free(HC_SR04_TRIG_PIN);
        gpio_free(HC_SR04_ECHO_PIN);
        printk(KERN_INFO "Bye test module sys!\n");
}

module_init(test_sys_init);
module_exit(test_sys_exit);
MODULE_LICENSE("GPL");
