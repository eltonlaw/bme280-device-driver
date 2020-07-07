#!/usr/bin/env python 
import argparse
import os
import sys
import subprocess

MODULE = "bme280"

parser = argparse.ArgumentParser(description="Setup and teardown of driver")
parser.add_argument('cmd', type=str, nargs='+',
                    help='One of [start|stop|restart|unregister-char-device]')

def call(cmd):
    return subprocess.call(cmd, shell=True)

def noop(argv):
    pass

def _get_major_numbers(name):
    """ Reads /proc/devices for loaded modules of name {MODULE} and returns all
    hits as a list of numbers"""
    numbers = []
    with open("/proc/devices", "r") as f:
        line = "placeholder"
        while line != "":
            line = f.readline().strip()
            if name in line:
                n, _ = line.split(" ")
                numbers.append(n)
    return numbers

def stop(argv):
    """ Unloads device driver (runs operations in reverse direction of initialization)

    - Remove module from the kernel
    """
    call(f"sudo rm /dev/{MODULE}*")
    call(f"sudo rmmod {MODULE}")
    print("Unloading BME280 module...")

def restart(argv):
    """ Helper function to force reload the driver"""
    stop(argv)
    start(argv)

def unregister_char_device(major_numbers):
    """Unregisters up module's registered character devices if for some reason there
    are hanging records in /proc/devices"""
    # If no major numbers were passed in from the command line, by default
    # get all major numbers of pre-existing loaded modules
    if len(major_numbers) == 0:
        major_numbers = _get_major_numbers(MODULE)

    # If there are no numbers to unregister, there's nothing to do
    if len(major_numbers) == 0:
        print(f"No character devices for {MODULE} to unregister")
        sys.exit(0)

    # Loads and unloads the devtools module passsing in `unregister-char-device`
    # to run the logic that unregisters character devices
    for device_number in major_numbers:
        print("Unregistering major number: ", device_number)
        call(f"sudo insmod devtools.ko cmd=unregister-char-device major={device_number}")
        call(f"sudo rmmod devtools")

def start(argv):
    """ Loads device driver into kernel

    - Inserts module into kernel if it exists
    """
    print("Loading BME280 module...")

    preexisting_major_numbers = _get_major_numbers(MODULE)

    # Warn if there are already modules loaded, shouldn't happen if there are no bugs
    if len(preexisting_major_numbers) > 0:
        print("WARNING: Preexisting major numbers already exist for this module:",
                preexisting_major_numbers)

    if not os.path.exists(f"{MODULE}.ko"):
        print(f"ERROR: '{MODULE}.ko' file doesn't exist. Run `make` first")
        sys.exit(0)
    call(f"sudo insmod {MODULE}.ko")

    # Make character special file for each major number that has the module prefix
    for i, major in enumerate(_get_major_numbers(MODULE)):
        special_file = f"/dev/{MODULE}{i}"
        print(f"Making character special file: {special_file} for major {major}") 
        call(f"sudo mknod {special_file} c {major} {i}")
    call(f"sudo chgrp wheel /dev/{MODULE}*")
    call(f"sudo chmod 664 /dev/{MODULE}*")

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
