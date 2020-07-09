#include <asm/current.h>    // current
#include <linux/uaccess.h>
#include <linux/cdev.h>     // cdev
#include <linux/fs.h>       // register_chrdev_region
#include <linux/init.h>
#include <linux/kdev_t.h>   // MKDEV
#include <linux/vermagic.h> // UTS_RELEASE
#include <linux/gpio.h>
#include <linux/module.h>   // THIS_MODULE
#include <linux/sched.h>    // task_struct
#include <linux/types.h>    // dev_t
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

static int n_devices = 1;
static int major; // If `alloc_chrdev_region` is successful this will be assigned a value
static int minor = 0;
static char* name = "bme280";
static struct cdev* cdev1 = NULL;

static int i2c_bus_number = 1;
module_param(i2c_bus_number, int, S_IRUGO);
MODULE_PARM_DESC(i2c_bus_number,
    "Original pi uses port 0, everything else uses 1");

/* register offsets */
#define BME280_I2C_BUS_ADDRESS 0x76

struct bme280_dev {
	struct cdev cdev;
	struct i2c_adapter i2c_adap;
	struct i2c_client *i2c_client;
    int x;
};

static void bme280_exit(void) {
    dev_t dev1;
    printk(KERN_ALERT "BME280 - Stopping device driver\n");

    if (cdev1 != NULL) {
        printk(KERN_ALERT "BME280 - Removing cdev1 from system");
        cdev_del(cdev1);
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

const struct i2c_board_info bme280_i2c_board_info = {
    I2C_BOARD_INFO("bme280", BME280_I2C_BUS_ADDRESS),
};

/* Used by `depmod` to generate map files. Used for hotplugging (not sure if
 * this is actually needed since we're using GPIO pins) */
// MODULE_DEVICE_TABLE(i2c, bme280_i2c_board_info);

int bme280_open(struct inode *inode, struct file *filp) {
	struct bme280_dev *dev;
    struct i2c_client *i2c_client;
    printk(KERN_ALERT "BME280 - `open` called");

    /* setup i2c client */
    i2c_client = i2c_new_device(i2c_get_adapter(1), &bme280_i2c_board_info);

    /* Create an instance of a bme280_dev and add the cdev putting everything
     * into private_data so we can access it from the read later */
	dev = container_of(inode->i_cdev, struct bme280_dev, cdev);
    dev->i2c_client = i2c_client;
    filp->private_data = dev;

    return 0;
}

int bme280_release(struct inode *inode, struct file *filp) {
	struct bme280_dev *dev;
    printk(KERN_ALERT "BME280 - `release` called");

	dev = container_of(inode->i_cdev, struct bme280_dev, cdev);
    if (dev->i2c_client) {
        printk(KERN_ALERT "BME280 - unregistering i2c_client");
        i2c_unregister_device(dev->i2c_client);
    }

    return 0;
}

ssize_t bme280_read (struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos) {
    struct bme280_dev* dev = filp->private_data;
    int err;
    unsigned long copied;
    /* i2c payload is 2 bytes: (address, value)*/
    int bufsize = 2;
    unsigned char data[2] = {0};

    printk(KERN_ALERT "BME280 - `read` called, count=%d", count);

    err = i2c_master_recv(dev->i2c_client, data, bufsize);
    if (err < 0) {
        printk(KERN_ALERT "BME280 - `i2c_master_recv` error %d", err);
        goto exit;
    }

    copied = copy_to_user(buf, data, bufsize);
    if (copied) {
        err = -EFAULT;;
        printk(KERN_ALERT "BME280 - `copy_to_user` error %d", err);
        goto exit;
    }
    printk(KERN_ALERT "BME280 - data=[%x %x],copied=%lu", data[0], data[1], copied);
    return 0;

exit:
    return err;
}

struct file_operations bme280_fops = {
	.owner = THIS_MODULE, /* Links file operations to this module */
    .poll = bme280_poll,
    .read = bme280_read,
    .open = bme280_open,
    .release = bme280_release,
};

static int __init bme280_init(void) {
    dev_t dev1;
    int err;
    printk(KERN_ALERT "BME280 - Initializing device driver. kernel=%s,parent_process=\"%s\",pid=%i\n",
        UTS_RELEASE, current->comm, current->pid);

    /* Register char device major number dynamically */
    err = alloc_chrdev_region(&dev1, minor, n_devices, name);
    if (err < 0) {
        printk(KERN_ALERT "BME280 - Can't get major number\n");
        goto exit;
    }
    major = MAJOR(dev1);
    printk(KERN_ALERT "BME280 - Assigned major number=%d\n", major);

    /* Setup cdev struct and add to system */
    printk(KERN_ALERT "BME280 - Allocating cdev");
    cdev1 = cdev_alloc();
    cdev_init(cdev1, &bme280_fops);
	cdev1->owner = THIS_MODULE;
    cdev1->ops = &bme280_fops;
    err = cdev_add(cdev1, dev1, 1);
    if (err < 0) {
        printk(KERN_ALERT "BME280 - Error encountered adding char device to system");
        goto exit;
    }

    printk(KERN_ALERT "BME280 - FINISHED INITIALIZING");
    return 0;

exit:
    bme280_exit();
    return err;
}

module_init(bme280_init);
module_exit(bme280_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Elton Law");
MODULE_DESCRIPTION("Bosch BME280 humidity and pressure sensor driver");
