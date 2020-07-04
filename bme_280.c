#include <asm/current.h> // current
#include <linux/init.h>
#include <linux/vermagic.h> // UTS_RELEASE
#include <linux/module.h>
#include <linux/sched.h> // task_struct

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("eltonlaw")
MODULE_VERSION("0.0.1")
MODULE_DESCRIPTION("Character device driver for BME280")

// These defaults are for the GPIO pins of a raspberry pi
static int scl = 11;
static int sda = 13;
module_param(scl;, int, S_IRUGO);
module_param(sda, int, S_IRUGO);

static int __init main_init(void) {
    printk(KERN_ALERT "Initializing BME280 device driver for kernel %s\n", UTS_RELEASE);
	printk(KERN_INFO "The process is \"%s\" (pid %i)\n",
			current->comm, current->pid);
    return 0;
}

static void main_exit(void) {
    printk(KERN_ALERT "Stopping BME280 device driver\n");
}

module_init(main_init);
module_exit(main_exit);
