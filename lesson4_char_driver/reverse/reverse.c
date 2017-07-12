#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#define DEVICE_NAME "reverse"
#define CLASS_NAME  "globallogic"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir Zahradnik");
MODULE_DESCRIPTION("Character Linux driver to output reverse string as received on input.");
MODULE_VERSION("0.1");

static int majorNumber;
static dev_t dev_no;
static char message[256] = {0};


static int __init reverse_init(void) {
  int ret;
  printk(KERN_INFO "ReverseLKM: initializing module\n");

  // Dynamically allocate major device number
  ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);

  if (ret < 0) {
      printk(KERN_ALERT "ReverseLKM: failed to register a major number\n");
      return ret;
  }

  majorNumber = MAJOR(dev_no);
  printk(KERN_INFO "ReverseLKM: registered correctly with major number %d\n", majorNumber);

  return 0;
}

static void __exit reverse_exit(void) {
  unregister_chrdev_region(majorNumber, 1);
  printk(KERN_INFO "ReverseLKM: module unloaded!\n");
}

module_init(reverse_init);
module_exit(reverse_exit);
