#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/device.h>

#define DEVICE_NAME "storageguard"
#define BUFFER_SIZE 256

static int major_number;
static char device_buffer[BUFFER_SIZE];
static size_t buffer_length;
static DEFINE_MUTEX(buffer_mutex);

static struct class *storageguard_class;
static struct device *storageguard_device;


/*
 * Automatically set /dev/storageguard permissions to 0666.
 * This removes the need to run chmod manually.
 */
static char *storageguard_devnode(const struct device *dev,
                                  umode_t *mode)
{
    if (mode)
        *mode = 0666;

    return NULL;
}


static int storageguard_open(struct inode *inode, struct file *file)
{
    pr_info("StorageGuard: device opened\n");
    return 0;
}


static int storageguard_release(struct inode *inode, struct file *file)
{
    pr_info("StorageGuard: device closed\n");
    return 0;
}


static ssize_t storageguard_read(struct file *file,
                                 char __user *user_buffer,
                                 size_t length,
                                 loff_t *offset)
{
    size_t to_copy;
    ssize_t result;

    if (mutex_lock_interruptible(&buffer_mutex))
        return -ERESTARTSYS;

    if (*offset >= buffer_length) {
        result = 0;
        goto out;
    }

    to_copy = min(length, buffer_length - (size_t)*offset);

    if (copy_to_user(user_buffer,
                     device_buffer + *offset,
                     to_copy)) {
        result = -EFAULT;
        goto out;
    }

    *offset += to_copy;
    result = to_copy;

out:
    mutex_unlock(&buffer_mutex);
    return result;
}


static ssize_t storageguard_write(struct file *file,
                                  const char __user *user_buffer,
                                  size_t length,
                                  loff_t *offset)
{
    size_t to_copy;
    ssize_t result;

    if (mutex_lock_interruptible(&buffer_mutex))
        return -ERESTARTSYS;

    to_copy = min(length, (size_t)(BUFFER_SIZE - 1));

    if (copy_from_user(device_buffer,
                       user_buffer,
                       to_copy)) {
        result = -EFAULT;
        goto out;
    }

    device_buffer[to_copy] = '\0';
    buffer_length = to_copy;

    pr_info("StorageGuard: received %zu bytes\n", to_copy);

    result = to_copy;

out:
    mutex_unlock(&buffer_mutex);
    return result;
}


static const struct file_operations storageguard_fops = {
    .owner = THIS_MODULE,
    .open = storageguard_open,
    .release = storageguard_release,
    .read = storageguard_read,
    .write = storageguard_write,
};


static int __init storageguard_init(void)
{
    int result;

    /*
     * Dynamically allocate a character-device major number.
     */
    major_number = register_chrdev(0, DEVICE_NAME, &storageguard_fops);

    if (major_number < 0) {
        pr_err("StorageGuard: character device registration failed\n");
        return major_number;
    }

    pr_info("StorageGuard: registered with major number %d\n",
            major_number);


    /*
     * Create /sys/class/storageguard
     */
    storageguard_class = class_create(DEVICE_NAME);

    if (IS_ERR(storageguard_class)) {
        result = PTR_ERR(storageguard_class);

        pr_err("StorageGuard: class creation failed\n");

        unregister_chrdev(major_number, DEVICE_NAME);

        return result;
    }

    /*
     * Make /dev/storageguard automatically.
     */
    storageguard_class->devnode = storageguard_devnode;

    storageguard_device = device_create(
        storageguard_class,
        NULL,
        MKDEV(major_number, 0),
        NULL,
        DEVICE_NAME
    );

    if (IS_ERR(storageguard_device)) {
        result = PTR_ERR(storageguard_device);

        pr_err("StorageGuard: device creation failed\n");

        class_destroy(storageguard_class);
        unregister_chrdev(major_number, DEVICE_NAME);

        return result;
    }

    pr_info("StorageGuard: device created at /dev/%s\n",
            DEVICE_NAME);

    return 0;
}


static void __exit storageguard_exit(void)
{
    device_destroy(
        storageguard_class,
        MKDEV(major_number, 0)
    );

    class_destroy(storageguard_class);

    unregister_chrdev(major_number, DEVICE_NAME);

    pr_info("StorageGuard: driver unloaded\n");
}


module_init(storageguard_init);
module_exit(storageguard_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shashank Kumar");
MODULE_DESCRIPTION("StorageGuard character-device driver");