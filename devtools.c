/// # devtools
///
/// This is a small module that does miscellaneous things when loaded. Using it
/// is basically to load it and then instantly unload  it:
/// 
///     sudo insmod devtools.ko <PARAMETERS> && sudo rmmod devtools
///
/// The main dispatch happens on the cmd parameter, here's the list of cases:
/// 
/// 1. "unregister-char-device": Runs `unregister_chrdev_region` on parameters `major` and `minor`
///
/// ## unregister-char-device
///
/// For cleaning up registered character devices if for some reason
/// there are hanging registered devices in /proc/devices:
///
///     sudo insmod devtools.ko cmd=unregister-char-device major=235 && sudo rmmod devtools
#include <linux/fs.h>       // unregister_chrdev_region
#include <linux/kdev_t.h>   // MKDEV
#include <linux/module.h>
#include <linux/types.h>    // dev_t

static char* cmd = "cmd";
static int major = 0;
static int minor = 0;
static int n_devices = 1;

module_param(cmd, charp, S_IRUGO);
// unregister-char-device specific params
module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);
module_param(n_devices, int, S_IRUGO);

MODULE_LICENSE("GPL v2");

void unregister_char_device(void) {
    dev_t dev;
    if (major > 0) {
        printk(KERN_NOTICE "[DEV-TOOLING] unregister-char-device: Unregistering char device %d\n", major);
        dev = MKDEV(major, minor);
        unregister_chrdev_region(dev, n_devices);
    } else {
        printk(KERN_WARNING "[DEV-TOOLING] unregister-char-device: Invalid major device number passed in: %d\n", major);
    }
}

static int __init main_init(void) {
    if (strcmp(cmd, "unregister-char-device") == 0) {
        unregister_char_device();
    } else {
        printk(KERN_ALERT "command invalid: `%s`", cmd);
    }
    return 0;
}

// an exit needs to be defined so that this can be removed
static void main_exit(void) {}

module_init(main_init);
module_exit(main_exit);
