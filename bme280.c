#include <asm/current.h>    // current
#include <linux/cdev.h>     // cdev
#include <linux/fs.h>       // register_chrdev_region
#include <linux/init.h>
#include <linux/kdev_t.h>   // MKDEV
#include <linux/vermagic.h> // UTS_RELEASE
#include <linux/gpio.h>
#include <linux/module.h>   // THIS_MODULE
#include <linux/sched.h>    // task_struct
#include <linux/types.h>    // dev_t

// These defaults are for the GPIO pins of a raspberry pi
// static int sda = 2;
// static int scl = 3;
// module_param(sda, int, S_IRUGO);
// module_param(scl, int, S_IRUGO);

int n_devices = 1;
int major; // If `alloc_chrdev_region` is successful this will be assigned a value
int minor = 0;
char* name = "bme280";
struct cdev* cdev1 = NULL;

/// module main exit function
static void bme280_exit(void) {
    dev_t dev1;
    printk(KERN_ALERT "BME280 - Stopping device driver\n");

    if (cdev1 != NULL) {
        printk(KERN_ALERT "BME280 - Removing cdev1 from system");
        cdev_del(cdev1);
    } else {
        printk(KERN_ALERT "BME280 - Skipping cdev_del");
    }

    if (major > 0) {
        printk(KERN_ALERT "BME280 - Unregistering char device %d\n", major);
        dev1 = MKDEV(major, minor);
        unregister_chrdev_region(dev1, n_devices);
    }

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

int bme280_release(struct inode *inode, struct file *filp) { return 0;
}

struct file_operations bme280_fops = {
	.owner = THIS_MODULE, // Links file operations to this module
    .poll = bme280_poll,
    .read = bme280_read,
    .open = bme280_open,
    .release = bme280_release,
};

/// module main init function
static int __init bme280_init(void) {
    dev_t dev1;
    printk(KERN_ALERT "BME280 - Initializing device driver. kernel=%s,parent process=\"%s\",pid=%i\n", UTS_RELEASE, current->comm, current->pid);

    // Register char device major number dynamically
    if (alloc_chrdev_region(&dev1, minor, n_devices, name) == 0) {
        major = MAJOR(dev1);
        printk(KERN_ALERT "BME280 - Assigned major number=%d\n", major);
    } else {
        printk(KERN_ALERT "BME280 - Can't get major number\n");
        bme280_exit();
    }

    // Setup cdev struct and add to system
    printk(KERN_ALERT "BME280 - Allocating cdev");
    cdev1 = cdev_alloc( );
	cdev1->owner = THIS_MODULE;
    cdev1->ops = &bme280_fops;
    if (cdev_add(cdev1, dev1, 1) < 0) {
        printk(KERN_ALERT "BME280 - Error encountered adding char device to system");
        bme280_exit();
    }
    printk(KERN_ALERT "BME280 - FINISHED INITIALIZING");

    return 0;
}

module_init(bme280_init);
module_exit(bme280_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Elton Law");
MODULE_DESCRIPTION("Bosch BME280 humidity and pressure sensor driver");
