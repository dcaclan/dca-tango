// SPDX-License-Identifier: GPL-2.0
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/security.h>
#include <linux/compat.h>
#include <linux/thread_info.h>
#include <linux/dirent.h>
#include "tango32.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
    #define get_file_from_fd(f) fd_file(f)
#else
    #define get_file_from_fd(f) ((f).file)
#endif

static bool is_32bit(void) {
    return test_thread_flag(TIF_32BIT);
}

static void set_32bit(bool val) {
    if (val) set_thread_flag(TIF_32BIT);
    else clear_thread_flag(TIF_32BIT);
}

static long tango32_get_version(struct tango32_abi_version __user *argp) {
    struct tango32_abi_version abi = { .major = TANGO32_ABI_MAJOR, .minor = TANGO32_ABI_MINOR };
    set_32bit(true);
    bool compat = in_compat_syscall();
    set_32bit(false);
    if (!compat) return -EIO;
    return copy_to_user(argp, &abi, sizeof(abi)) ? -EFAULT : 0;
}

static long tango32_set_mm(struct tango32_mm __user *argp) {
    struct mm_struct *mm = current->mm;
    struct tango32_mm f;
    if (copy_from_user(&f, argp, sizeof(f))) return -EFAULT;
    if (mmap_read_lock_killable(mm)) return -EINTR;
    spin_lock(&mm->arg_lock);
    mm->start_code = f.start_code; mm->end_code = f.end_code;
    mm->start_data = f.start_data; mm->end_data = f.end_data;
    mm->start_brk = f.start_brk; mm->brk = f.brk;
    mm->start_stack = f.start_stack; mm->arg_start = f.arg_start;
    mm->arg_end = f.arg_end; mm->env_start = f.env_start;
    mm->env_end = f.env_end;
    spin_unlock(&mm->arg_lock);
    mmap_read_unlock(mm);
    return 0;
}

static long tango32_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    void __user *argp = (void __user *)arg;
    if (is_32bit()) return -EINVAL;
    switch (cmd) {
        case TANGO32_GET_VERSION: 
            return tango32_get_version(argp);
        case TANGO32_SET_MM: 
            return tango32_set_mm(argp);
        default:
            return -ENOIOCTLCMD;
    }
}

static const struct file_operations tango32_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = tango32_ioctl,
    .compat_ioctl = tango32_ioctl,
};

static struct miscdevice tango32_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "tango32",
    .fops = &tango32_fops,
    .mode = 0666,
};

module_misc_device(tango32_device);
MODULE_LICENSE("GPL");
