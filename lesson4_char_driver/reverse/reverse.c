#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h> 
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#define  DEVICE_NAME "reverse"
#define  CLASS_NAME  "training"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir Zahradnik");
MODULE_DESCRIPTION("Character Linux driver to output reverse string as received on input.");
MODULE_VERSION("0.1");

static int    majorNumber;
static dev_t  dev_no;
static char   message[256] = {0};
static struct cdev* kernel_cdev;
static struct class*  reverseClass  = NULL;
static struct device* reverseDevice = NULL;

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int __init reverse_init(void) {
    int ret;
    printk(KERN_INFO "ReverseLKM: initializing module\n");

    // Allocate cdev struct
    kernel_cdev = cdev_alloc();
    kernel_cdev->ops = &fops;
    kernel_cdev->owner = THIS_MODULE;

    // Dynamically allocate major device number
    if ((ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME)) < 0) {
        printk(KERN_ALERT "ReverseLKM: failed to register a major number\n");
        return ret;
    }

    majorNumber = MAJOR(dev_no);
    printk(KERN_INFO "ReverseLKM: registered correctly with major number %d\n", majorNumber);

    // Register the device class
    reverseClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(reverseClass)) {
        unregister_chrdev_region(majorNumber, 1);
        printk(KERN_ALERT "ReverseLKM: failed to register device class\n");
        return PTR_ERR(reverseClass);
    }
    printk(KERN_INFO "ReverseLKM: device class registered correctly\n");

    // Register the device driver -- create node under /dev
    reverseDevice = device_create(reverseClass, NULL, dev_no, NULL, DEVICE_NAME);
    if (IS_ERR(reverseDevice)) {
        unregister_chrdev_region(majorNumber, 1);
        printk(KERN_ALERT "ReverseLKM: failed to create the device\n");
        return PTR_ERR(reverseDevice);
    }
    printk(KERN_INFO "ReverseLKM: device node created correctly\n");

    // Pass cdev struct to kernel. Do it as a last init step, since module
    // should be fully initialized before handling cdev to kernel
    if((ret = cdev_add(kernel_cdev, dev_no, 1)) < 0) {
        printk(KERN_INFO "Unable to allocate cdev");
        return ret;
    }

    return 0;
}

static void __exit reverse_exit(void) {
    cdev_del(kernel_cdev);
    device_destroy(reverseClass, dev_no);
    class_unregister(reverseClass);
    class_destroy(reverseClass);
    unregister_chrdev_region(majorNumber, 1);
    printk(KERN_INFO "ReverseLKM: module unloaded!\n");
}

static int dev_open(struct inode *inodep, struct file *filep){
    // TODO: Implement code
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    // TODO: Implement code
    return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
    // TODO: Implement code
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep){
    // TODO: Implement code
    return 0;
}

module_init(reverse_init);
module_exit(reverse_exit);
