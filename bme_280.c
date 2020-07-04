#include <asm/current.h>    // current
#include <linux/fs.h>       // register_chrdev_region
#include <linux/init.h>
#include <linux/kdev_t.h>   // MKDEV
#include <linux/vermagic.h> // UTS_RELEASE
#include <linux/module.h>
#include <linux/sched.h>    // task_struct
#include <linux/types.h>    // dev_t

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("eltonlaw");
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Character device driver for BME280");

// These defaults are for the GPIO pins of a raspberry pi
static int sda = 2;
static int scl = 3;
module_param(sda, int, S_IRUGO);
module_param(scl;, int, S_IRUGO);

dev_t* dev1 = NULL;
int n_devices = 1;
int minor = 0;
char* name = "bme-280";

static void main_exit(void) {
    printk(KERN_ALERT "Stopping BME280 device driver\n");
    if (dev1 != NULL) {
        unregister_chrdev_region(*dev1, n_devices);
    }
}

static int __init main_init(void) {
    printk(KERN_ALERT "Initializing BME280 device driver for kernel %s\n",UTS_RELEASE);
	printk(KERN_INFO "The process is \"%s\" (pid %i)\n",
			current->comm, current->pid);

    // Obtain device numbers
    if (alloc_chrdev_region(dev1, minor, n_devices, name) != 0) {
        main_exit();
    }
    return 0;
}

module_init(main_init);
module_exit(main_exit);
