#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h> 
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/mutex.h> // Limit LKM use to only one concurrent process
#define  DEVICE_NAME "reverse"
#define  CLASS_NAME  "training"
#define  MAX_MSG_SIZE 255

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir Zahradnik");
MODULE_DESCRIPTION("Character Linux driver to output reverse string as received on input.");
MODULE_VERSION("0.1");

static int    majorNumber;
static dev_t  dev_no;
static char   message[MAX_MSG_SIZE + 1] = {0}; // Remember to store \0 character
static short  size_of_message;
static int    numberOpens = 0;
static struct cdev* kernel_cdev;
static struct class*  reverseClass  = NULL;
static struct device* reverseDevice = NULL;

static DEFINE_MUTEX(reverse_mutex);     // Macro to declare a new mutex

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
    printk(KERN_INFO "ReverseLKM: Initializing module\n");

    // Allocate cdev struct
    kernel_cdev = cdev_alloc();
    kernel_cdev->ops = &fops;
    kernel_cdev->owner = THIS_MODULE;

    // Dynamically allocate major device number
    if ((ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME)) < 0) {
        printk(KERN_ALERT "ReverseLKM: Failed to register a major number\n");
        return ret;
    }

    majorNumber = MAJOR(dev_no);
    printk(KERN_INFO "ReverseLKM: Registered correctly with major number %d\n", majorNumber);

    // Register the device class
    reverseClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(reverseClass)) {
        unregister_chrdev_region(majorNumber, 1);
        printk(KERN_ALERT "ReverseLKM: Failed to register device class\n");
        return PTR_ERR(reverseClass);
    }
    printk(KERN_INFO "ReverseLKM: Device class registered correctly\n");

    // Register the device driver -- create node under /dev
    reverseDevice = device_create(reverseClass, NULL, dev_no, NULL, DEVICE_NAME);
    if (IS_ERR(reverseDevice)) {
        class_destroy(reverseClass);
        unregister_chrdev_region(majorNumber, 1);
        printk(KERN_ALERT "ReverseLKM: Failed to create the device\n");
        return PTR_ERR(reverseDevice);
    }
    printk(KERN_INFO "ReverseLKM: Device node created correctly\n");

    // Pass cdev struct to kernel. Do it as a last init step, since module
    // should be fully initialized before handling cdev to kernel
    if((ret = cdev_add(kernel_cdev, dev_no, 1)) < 0) {
	    device_destroy(reverseClass, dev_no);
	    class_unregister(reverseClass);
        class_destroy(reverseClass);
        unregister_chrdev_region(majorNumber, 1);
        printk(KERN_INFO "ReverseLKM: Unable to allocate cdev");

        return ret;
    }

    mutex_init(&reverse_mutex);
    return 0;
}

static void __exit reverse_exit(void) {
    mutex_destroy(&reverse_mutex);
    cdev_del(kernel_cdev);
    device_destroy(reverseClass, dev_no);
    class_unregister(reverseClass);
    class_destroy(reverseClass);
    unregister_chrdev_region(majorNumber, 1);
    printk(KERN_INFO "ReverseLKM: Module unloaded!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    // Try to acquire the mutex (return 0 on fail)
    if(!mutex_trylock(&reverse_mutex)) {
        printk(KERN_ALERT "ReverseLKM: Device in use by another process");
        return -EBUSY;
    }
    numberOpens++;
    printk(KERN_INFO "ReverseLKM: Device has been opened %d time(s)\n", numberOpens);
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    short msglen = size_of_message;

    // Reverse the text
    char reverse_msg[MAX_MSG_SIZE + 1] = {0}

    for (int i = 0; i < msglen; ++i) {
        reverse_msg[i] = message[msglen - i];
    }
    reverse_msg[msglen] = '\0';

   // copy_to_user has the format (* to, *from, length) and returns 0 on success
   int error_count = copy_to_user(buffer, reverse_msg, size_of_message);

   if (error_count == 0) {
      printk(KERN_INFO "ReverseLKM: Sent %d characters to the user\n", size_of_message);
      size_of_message = 0;
      return msglen; // clear the position to the start and return 0
   } else {
      printk(KERN_INFO "ReverseLKM: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;      // Failed -- return a bad address message (i.e. -14)
   }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    if (len > MAX_MSG_SIZE) {
        printk(KERN_INFO "ReverseLKM: Input message exceeds max length\n");
        return -EFAULT;
    }

    // copy_from_user has the format (* to, *from, length) and returns 0 on success
    int error_count = copy_from_user(message, buffer, len);

    if (error_count == 0) {
      printk(KERN_INFO "ReverseLKM: Received %zu characters from the user\n", len);
      return (size_of_message = len);
   } else {
      printk(KERN_INFO "ReverseLKM: Failed to receive %d characters from the user\n", error_count);
      return -EFAULT;      // Failed -- return a bad address message (i.e. -14)
   }
}

static int dev_release(struct inode *inodep, struct file *filep) {
    // Release the mutex (i.e., lock goes up)
    mutex_unlock(&reverse_mutex);
    printk(KERN_INFO "ReverseLKM: Device successfully closed\n");
    return 0;
}

module_init(reverse_init);
module_exit(reverse_exit);
