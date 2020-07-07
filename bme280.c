#include <asm/current.h>    // current
#include <linux/fs.h>       // register_chrdev_region
#include <linux/init.h>
#include <linux/kdev_t.h>   // MKDEV
#include <linux/vermagic.h> // UTS_RELEASE
#include <linux/module.h>   // THIS_MODULE
#include <linux/sched.h>    // task_struct
#include <linux/types.h>    // dev_t

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("eltonlaw");
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Character device driver for BME280");

// These defaults are for the GPIO pins of a raspberry pi
// static int sda = 2;
// static int scl = 3;
// module_param(sda, int, S_IRUGO);
// module_param(scl, int, S_IRUGO);

int n_devices = 1;
int major; // If `alloc_chrdev_region` is successful this will be assigned a value
int minor = 0;
char* name = "bme280";

static void main_exit(void) {
    dev_t dev1;
    printk(KERN_ALERT "BME280 - Stopping device driver\n");

    if (major > 0) {
        printk(KERN_ALERT "BME280 - Unregistering char device %d\n", major);
        dev1 = MKDEV(major, minor);
        unregister_chrdev_region(dev1, n_devices);
    }
}

static int __init main_init(void) {
    dev_t dev1;
    printk(KERN_ALERT "BME280 - Initializing device driver. kernel=%s,parent process=\"%s\",pid=%i\n",
            UTS_RELEASE, current->comm, current->pid);

    // Register char device major number dynamically
    if (alloc_chrdev_region(&dev1, minor, n_devices, name) == 0) {
        major = MAJOR(dev1);
        printk(KERN_ALERT "BME280 - Assigned major number=%d\n", major);
    } else {
        printk(KERN_ALERT "BME280 - Can't get major number\n");
        main_exit();
    }

    return 0;
}

unsigned int bme280_poll (struct file* filp, struct poll_table_struct * pt) {
	unsigned int mask = 0;
    return mask;
}

ssize_t bme280_read (struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos) {
    ssize_t retval =0;
    return retval;
}

int bme280_open(struct inode *inode, struct file *filp) {
    return 0;
}

int bme280_release(struct inode *inode, struct file *filp) {
	return 0;
}

struct file_operations fops = {
	.owner = THIS_MODULE, // Links file operations to this module
    .poll = bme280_poll,
    .read = bme280_read,
    .open = bme280_open,
    .release = bme280_release,
};

module_init(main_init);
module_exit(main_exit);
