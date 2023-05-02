# Installation
You will have to compile the game yourself, as it's legally dubious for me to distribute actual executable files of the game

First of all verify that you:
- Have a Nspire CX (preferably II) model that has a <ins>low enough operating system</ins> to use the ndless runtime. See the [ndless repo](https://github.com/ndless-nspire/Ndless) for more information
- Are in possession of a sm64 rom file with a .z64 extension
- Have a pc to carry out the remaining compilation instructions

The building process may be a bit lengthy, please make sure not to skip a step.

## Step 1 - Environment
Ensure you are able to access a Ubuntu-like CLI, this will be easiest to use going forward.

Windows users can use WSL, which is what I've used, to achieve the same goal. You may want to run `cd ~` and `explorer.exe .` to get your bearings.

Ensure you have the needed base dependencies: `sudo apt install -y git build-essential pkg-config libusb-1.0-0-dev libsdl2-dev`

## Step 2 - Toolchain

Next you will need to install the ndless toolchain, to get the needed compilation tools. Use [this guide](https://hackspire.org/index.php/C_and_assembly_development_introduction) as a reference. You can stop when you reach the <i>2-minute tutorial</i> section

## Step 3 - Compilation

Once you have all the needed tools, you will compile the executable.

In an appropriate directory, run: `git clone https://github.com/rayzerbrain/sm64-nsp`

`cd sm64-nsp`

You should be in the root level of the respository. Place your `.z64` rom in this directory so the assets can be extracted from it.

Run `make` to compile, or `make -j4` to possibly build faster. Wait for it to complete.

Your executable (`sm64.us.f3dex2.tns`) should be located at `./build/us_nsp/`

## Step 4 - Installation

Find a proper file transfer system between your pc and your calculator, like the TI-Nspire CX Student Software, and follow the ndless installation instructions in the [ndless repo](https://github.com/ndless-nspire/Ndless)

For ease of access, create a root-level folder named `sm64` on your calculator, and transfer the executable to it.

## Step 5 - Finish

You're done! Once you run the executable, a config and save file file will be generated, you can only edit the config file by transferring it back to your pc first, however.

Be aware there are a few hiccups when running the game now and then. If attempting to run the executable shows a compatiblity error, you'll need to restart your calc with the reset button on the back (This will require you to install ndless again, too). Also note that running the game with the usb cable still attached can impact performance negatively.
