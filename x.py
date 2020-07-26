#!/usr/bin/env python3
"""Script for loading and unloading the module.

Check the available commands map for a list of the command line options available.
"""
import argparse
import glob
import os
import sys
import subprocess

MODULE = "bme280"

REMOTE_SSH_ALIAS = os.environ.get("REMOTE_SSH_ALIAS")
REMOTE_DIR = os.environ.get("REMOTE_DIR")
TMUX_SESSION_NAME = 0 # Hardcoded to 0

def call(cmd):
    """Run strings as sh commands"""
    return subprocess.call(cmd.split(" "))

def noop(argv):
    print(argv)
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

    - Remove character special files from /dev if they exist
    - Remove module from the kernel
    """
    special_char_files = glob.glob("/dev/bme280*")
    if len(special_char_files) > 0:
        subprocess.call(["rm"] + special_char_files)

    call(f"sync") # flushes buffered writes
    call(f"rmmod {MODULE}")
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
        call(f"sync") # flushes buffered writes
        call(f"insmod devtools.ko cmd=unregister-char-device major={device_number}")
        call(f"sync") # flushes buffered writes
        call(f"rmmod devtools")

def existing_su_group():
    with open("/etc/group", "r") as f:
        line  = "placeholder"
        while line != "":
            line = f.readline()
            if "wheel" in line:
                return "wheel"
    return "staff"

def start(argv):
    """ Loads device driver into kernel

    - Inserts module into kernel if it exists
    - Creates character special files
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
    call(f"sync") # flushes buffered writes
    call(f"insmod {MODULE}.ko")

    # Make character special file for each major number that has the module prefix
    for i, major in enumerate(_get_major_numbers(MODULE)):
        # /dev is conventionally where char devices are put
        special_file = f"/dev/{MODULE}{i}"
        su_group = existing_su_group()
        print(f"Making character special file: {special_file} for major {major}, assigned to {su_group}") 
        call(f"mknod {special_file} c {major} {i}")
        call(f"chgrp {su_group} {special_file}")
        call(f"chmod 664 {special_file}")

def test(argv):
    device = f"/dev/{MODULE}0"
    if os.path.exists(device):
        f = open(device, "r")
        print("Read:", f.read())
        print("Read:", f.read())
        print("Read:", f.read())
        print("Read:", f.read())
        f.close()
    else:
        print(f"ERROR: device doesn't exist")

def rinit(argv):
    new_session = f"new -d -s {TMUX_SESSION_NAME}"
    split_pane = "split-window -h"
    cd = f"send-keys -t {TMUX_SESSION_NAME}:0.0 'cd {REMOTE_DIR}/bme280-device-driver && clear' 'Enter'"
    start_journactl = "select-pane -t 1 \; send-keys 'sudo journalctl -f' 'Enter'"
    call(f"ssh -t {REMOTE_SSH_ALIAS} tmux {new_session} \; {split_pane} \; {cd} \; {start_journactl}")

def rtest(argv):
    """Send local repo to raspberry pi and run tests

    ...assumes a tmux session is already open with a name of 0"""
    # Copy local to remote
    call(f"rsync -a --delete . {REMOTE_SSH_ALIAS}:{REMOTE_DIR}/bme280-device-driver")
    # Run test in ssh session
    tmux_command = f"tmux send-keys -t {TMUX_SESSION_NAME}:0.0 'make && sudo ./x.py restart && ./x.py test' 'Enter'"
    call(f"ssh -t {REMOTE_SSH_ALIAS} {tmux_command}")

available_commands = {
    "start": start, # load module into kernel, make special file
    "stop": stop, # unload module from kernel, rm special file
    "restart": restart,
    # DEV COMMANDS
    "unregister-char-device": unregister_char_device,
    "noop": noop, # does nothing, used for testing this script
    "test": test, # test to read from special file
    "rinit": rinit, # start a tmux session on remote
    "rtest": rtest, # rsync local to remote and run `test`
}

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Setup and teardown of driver")
    top_level_cmds = sorted(list(available_commands.keys()))
    parser.add_argument("cmd", type=str, nargs="+",
                        help=f"One of [{', '.join(top_level_cmds)}]")
    args = parser.parse_args()
    cmd = available_commands.get(args.cmd[0])
    if cmd is not None:
        cmd(args.cmd[1:])
    else:
        parser.print_help()
        print(f"ERROR: '{args.cmd}' is not a valid command")
