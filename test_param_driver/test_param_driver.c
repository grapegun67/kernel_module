#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#if 1

#define ARRNUM  5

int     numPARM, arr_numPARM[ARRNUM];
char    *namePARM;
int     cb_numPARM;

module_param(numPARM, int, S_IRUSR | S_IWUSR);
module_param(namePARM, charp, S_IRUSR | S_IWUSR);
module_param_array(arr_numPARM, int, NULL, S_IRUSR | S_IWUSR);

int callback_test(const char *tmp, const struct kernel_param *kp)
{
        int ret = param_set_int(tmp, kp);

        if (ret == 0)
        {
                printk(KERN_INFO "call back START\n");
                printk(KERN_INFO "New Value(cb_numPARM): [%d]\n", cb_numPARM);
                return (0);
        }

        else
                return (-1);

}

const struct kernel_param_ops test_ops =
{
        .set = &callback_test,
        .get = &param_get_int,
};

module_param_cb(cb_numPARM, &test_ops, &cb_numPARM, S_IRUSR | S_IWUSR);

#endif

static int __init test_func_init(void)
{
        int ii;

        printk(KERN_INFO "Welcome test module func!\n");

        printk(KERN_INFO "numPARM: [%d]\nnamePARM:[%s]\ncb_numPARM:[%d]\n", numPARM, namePARM, cb_numPARM);
        for (ii = 0; ii < ARRNUM; ii++)
                printk(KERN_INFO "arr_numPARM[%d]: [%d]\n", ii, arr_numPARM[ii]);
        
        printk(KERN_INFO "Initialize Finished\n");

        return (0);
}

static void __exit test_func_exit(void)
{
        printk(KERN_INFO "Bye test module func!\n");
}

module_init(test_func_init);
module_exit(test_func_exit);
MODULE_LICENSE("GPL");
