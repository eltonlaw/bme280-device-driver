#!/usr/bin/env python 
import argparse
import os
import sys
from subprocess import call

MODULE = "bme280"

parser = argparse.ArgumentParser(description="Setup and teardown of driver")
parser.add_argument('cmd', type=str, nargs='+',
                    help='One of [start|stop|restart]')

def _get_major_numbers():
    """ Reads /proc/devices for loaded modules of name {MODULE} and returns all
    hits as a list of numbers"""
    numbers = []
    with open("/proc/devices", "r") as f:
        line = "placeholder"
        while line != "":
            line = f.readline().strip()
            if MODULE in line:
                n, _ = line.split(" ")
                numbers.append(n)
    return numbers

def start(argv):
    """ Loads device driver into kernel

    - Inserts module into kernel if it exists
    """
    print("Loading BME280 module...")
    if os.path.exists(f"{MODULE}.ko"):
        call(f"sudo insmod {MODULE}.ko", shell=True)
    else:
        print(f"ERROR: '{MODULE}.ko' file doesn't exist. Run `make` first")

def stop(argv):
    """ Unloads device driver

    - Remove module from the kernel
    """
    call(f"sudo rmmod {MODULE}", shell=True)
    print("Unloading BME280 module...")

def restart(argv):
    """ Helper function to force reload the driver"""
    stop()
    start()

def unregister_char_device(major_numbers):
    """Unregisters up module's registered character devices if for some reason there
    are hanging records in /proc/devices"""
    # If no major numbers were passed in from the command line, by default
    # get all major numbers of pre-existing loaded modules
    if len(major_numbers) == 0:
        major_numbers = _get_major_numbers()

    # If there are no numbers to unregister, there's nothing to do
    if len(major_numbers) == 0:
        print(f"No character devices for {MODULE} to unregister")
        sys.exit(0)

    # Loads and unloads the devtools module passsing in `unregister-char-device`
    # to run the logic that unregisters character devices
    for device_number in major_numbers:
        print("Unregistering major number: ", device_number)
        call(f"sudo insmod devtools.ko cmd=unregister-char-device major={device_number}", shell=True)
        call(f"sudo rmmod devtools", shell=True)

def noop(argv):
    pass

available_commands = {
    "start": start,
    "stop": stop,
    "restart": restart,
    "unregister-char-device": unregister_char_device,
    "noop": noop,
}

if __name__ == "__main__":
    args = parser.parse_args()
    cmd = available_commands.get(args.cmd[0])
    if cmd is not None:
        cmd(args.cmd[1:])
    else:
        parser.print_help()
        print(f"ERROR: '{args.cmd}' is not a valid command")
