// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include "tango32.h"

static long tango32_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    void __user *argp = (void __user *)arg;
    struct tango32_abi_version abi = { .major = TANGO32_ABI_MAJOR, .minor = TANGO32_ABI_MINOR };

    // Kernel 6.6 compatibility: Direct thread flag check
    if (test_thread_flag(TIF_32BIT)) return -EINVAL;

    if (cmd == TANGO32_GET_VERSION) {
        set_thread_flag(TIF_32BIT);
        bool c = in_compat_syscall();
        clear_thread_flag(TIF_32BIT);
        if (!c) return -EIO;
        if (copy_to_user(argp, &abi, sizeof(abi))) return -EFAULT;
        return 0;
    }

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

module_misc_device(t_dev);
MODULE_LICENSE("GPL");
