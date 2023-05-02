# Super Mario 64 Nspire Port

This is a port of SM64 to the TI-Nspire CX lineup. 

It is fully playable, albeit painfully slow, especially on a CX I. Play at your own risk of dying of old age.

![Emulated, at full resolution](https://user-images.githubusercontent.com/82989402/235566001-9cb138d9-3ce4-4ab1-86ed-3aff23919145.png)
![Game beaten with 100% completion, on actual hardware.](https://user-images.githubusercontent.com/82989402/235566406-f7c80234-1ef9-44b4-8bd6-a956d846431e.jpeg)

## Installation Instructions

See [Installing](./INSTALLING.md)

## Performance


Performance will likely never be that great, but here are a few ways to maximize playability:

 - Make sure the config option `enable_120p_mode` is set to `true`, which is the default. However, this will quarter resolution and make most text unreadable.
 - Set the config option `draw_sky` to `false` to avoid drawing the skybox (you can pretend it's always nighttime)
 - Set `enable_fog` to `false` to disable fog (this is the default, it's never actually been tested when it's on, anyway)
 - Find the right `frameskip` value for you. The default value is 4, but you can increase it at the cost of precise maneuvering.

With base configuration, you should only expect around 4 FPS on average on a CX II.


## Controls

Controls cannot be edited via the config file, if you need to change the controls yourself you'll have to adjust the [according file](./src/pc/controller_entry_point.c)

The default controls are shown on launch, they are:

Start: enter

A Button: menu


B Button: del

Z Trigger: doc

Analog Controls: Touchpad

C Buttons (up, down left, right): 8, 2, 4, 6, accordingly

Escape exits the game, and pressing Control brings up a profiling screen, which shows your current FPS.


## Credits
 - https://github.com/fgsfdsfgs/sm64-port: Original fork; Software renderer adapted from here
 - https://github.com/ndless-nspire/Ndless: Nspire compile and runtime capabilities
 - https://github.com/nspire-emus/firebird: Emulator used extensively for testing and debugging purposes
 - https://github.com/Pi-Boy-314: Project idea
 - The almighty God from Whom all blessings flow
