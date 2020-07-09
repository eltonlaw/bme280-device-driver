# BME280

Linux device driver for the 4-pin variant (I2C only) of the [BME280](https://www.amazon.ca/gp/product/B07KYJNFMD/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1)

## Development

* Append `bme280` to `/etc/modules` to load at boot

### i2c

Enable in `raspi-config` and install tooling if it doesn't exist

    sudo apt-get install i2c-tools i2c-dev

To check that it's working:

    sudo i2cdetect -y 1

### `libgpiod`

    sudo apt update && sudo apt full-upgrade
    sudo apt-get install gpiod libgpiod-dev libgpiod-doc

    # Sanity check
    sudo gpiodetect # Prints info. The chip with 8 lines is the one we want
    sudo gpioset --mode=signal gpiochip1 4=1 # Pull GPIO4 high until SIGINT

### Setting up hot-reload (kind-of) on the raspberry pi

On the Pi make sure kernel headers are in `/lib/modules/<uname -r>/`, or, installing via `apt`:

    sudo apt-get install linux-headers

And locally have a `.envrc` file that looks like (with your own settings):

    export REMOTE_SSH_ALIAS=pi
    export REMOTE_DIR=/home/eltonlaw

Running `make rsync` will rsync over the entire current working directory, compile and reload the module. This assumes the Pi is configured with ssh and tmux is installed.

### Load module into kernel

    ./x.py start
    ./x.py stop
    ./x.py restart # Helper to run `stop` then `start`

Current progress, logs written to systemd journal. Here's some output from a python script with the associated logs

    # <<LOAD MODULE>>
    # Jul 09 18:56:33 raspberrypi sudo[6808]: pam_unix(sudo:session): session opened for user root by d0nkrs(uid=0)
    # Jul 09 18:56:33 raspberrypi kernel: BME280 - Stopping device driver
    # Jul 09 18:56:33 raspberrypi kernel: BME280 - Removing cdev1 from system
    # Jul 09 18:56:33 raspberrypi kernel: BME280 - Unregistering char device 240
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - Initializing device driver. kernel=4.19.118-v7+,parent_process="insmod",pid=6817
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - Assigned major number=240
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - Allocating cdev
    # Jul 09 18:56:34 raspberrypi sudo[6808]: pam_unix(sudo:session): session closed for user root
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - Initialized successfully

    file = open("/dev/bme2800")
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - `open` called

    f.read() # ret: ''
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - `read` called, count=8192
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - data=[57 8],copied=0

    f.read() # ret: ''
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - `read` called, count=8192
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - data=[0 51],copied=0

    f.read() # ret: ''
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - `read` called, count=8192
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - data=[78 90],copied=0

    f.close()  
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - `release` called
    # Jul 09 18:56:34 raspberrypi kernel: BME280 - unregistering i2c_client

## Credits

Portions of code taken directly from/inspired by [Linux Device Driver by Jonathan Corbet, Alessandro Rubini, and Greg Kroah-Hartman](https://lwn.net/Kernel/LDD3/) which has graciously been published online in its entirety and [the up-to-date examples by Javier Martinez Canillas et. al](https://github.com/martinezjavier/ldd3) from which the Makefile and more portions of code are taken from.
