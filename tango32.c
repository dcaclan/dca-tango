// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <linux/sched.h>

static long tango32_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    // 32-bit check logic
    if (test_thread_flag(TIF_32BIT)) return -EINVAL;
    
    // Minimal IOCTL to avoid struct padding issues
    return -ENOIOCTLCMD;
}

static const struct file_operations t_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = tango32_ioctl,
    .compat_ioctl = tango32_ioctl,
};

static struct miscdevice t_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "tango32",
    .fops = &t_fops,
    .mode = 0666,
};

static int __init t_init(void) {
    return misc_register(&t_dev);
}

static void __exit t_exit(void) {
    misc_deregister(&t_dev);
}

module_init(t_init);
module_exit(t_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DCA");
MODULE_INFO(intree, "Y");
