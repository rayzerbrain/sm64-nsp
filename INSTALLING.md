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

Ensure you have the needed base dependencies: 

`sudo apt-get update`

`sudo apt-get upgrade`

`sudo apt-get install -y git gcc build-essential pkg-config binutils wget python3-dev libgmp-dev libmpfr-dev libmpc-dev zlib1g-dev libboost-all-dev texinfo php`

## Step 2 - Toolchain

Next you will need to install the ndless toolchain, to get the needed compilation tools. I've adapted the required commands from [this guide](https://hackspire.org/index.php/C_and_assembly_development_introduction).

Building the toolchain may take a while, be prepared to have to wait for several minutes.

```
git clone --recursive https://github.com/ndless-nspire/Ndless.git
cd Ndless/ndless-sdk/toolchain/
./build_toolchain.sh
cd ../..
export PATH=$PATH:$(pwd)/ndless-sdk/toolchain/install/bin:$(pwd)/ndless-sdk/bin
make
```

Note that the changes made to your path are temporary and will only exist for your current login session. To make them permanent, you'll have to put the same command with absolute paths in a file such as your `~/.bashrc`

## Step 3 - Compilation

Once you have all the needed tools, you will compile the executable.

Change to an appropiate directory (`cd ~`)
Run: `git clone https://github.com/rayzerbrain/sm64-nsp`

`cd sm64-nsp`

You should be in the root level of the respository. Place your `.z64` rom file in this directory.

Run `make` to compile, or `make -j4` to possibly build faster. Wait for it to complete.

If needed, specify the version with the `VERSION` option, e.g. `make VERSION=jp`. Options are `us` (default), `jp`, `eu`, and `sh` 

Your executable (`sm64.<VERSION>.f3dex2.tns`) should be located at `./build/<VERSION>_nsp/`

## Step 4 - Installation

Find a proper file transfer system between your pc and your calculator, like the TI-Nspire CX Student Software, and follow the ndless installation instructions in the [ndless repo](https://github.com/ndless-nspire/Ndless)

For ease of access, create a root-level folder named `sm64` on your calculator, and transfer the executable to it.

## Step 5 - Finish

You're done! Once you run the executable, a config and save file file will be generated, you can only edit the config file by transferring it back to your pc first, however.

Be aware there are a few hiccups when running the game now and then. If attempting to run the executable shows a compatiblity error, you'll need to restart your calc with the reset button on the back (This will require you to install ndless again, too). Also note that running the game with the usb cable still attached can impact performance negatively.
