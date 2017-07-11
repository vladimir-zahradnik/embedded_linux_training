#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir Zahradnik");
MODULE_DESCRIPTION("Character Linux driver to output reverse string as received on input.");
MODULE_VERSION("0.1");

static int __init reverse_init(void){
   printk(KERN_INFO "Hello World from LKM!\n");
   return 0;
}

static void __exit reverse_exit(void){
   printk(KERN_INFO "Goodbye World from LKM!\n");
}

module_init(reverse_init);
module_exit(reverse_exit);
