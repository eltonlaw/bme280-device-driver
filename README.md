# BME280

Linux device driver for the 4-pin variant (I2C only) of the [BME280](https://www.amazon.ca/gp/product/B07KYJNFMD/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1)

## Development

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

## Credits

Portions of code taken directly from/inspired by [Linux Device Driver by Jonathan Corbet, Alessandro Rubini, and Greg Kroah-Hartman](https://lwn.net/Kernel/LDD3/) which has graciously been published online in its entirety and [the up-to-date examples by Javier Martinez Canillas et. al](https://github.com/martinezjavier/ldd3) from which the Makefile and more portions of code are taken from.
