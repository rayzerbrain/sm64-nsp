# Super Mario 64 DOS Port

This is a novelty port of the sm64-port to DOS. Do not expect it to be playable.

![Goddard](https://raw.githubusercontent.com/mkst/sm64-port/dos/media/screenshot_00.png)

![In-game](https://raw.githubusercontent.com/mkst/sm64-port/dos/media/screenshot_01.png)

## Building Instructions

### Docker

1. Install Docker
2. Build image: `docker build . -t sm64_dos`
3. Run build: `docker run --rm --mount type=bind,source="$(pwd)",destination=/sm64 -ti sm64_dos make -j4 ENABLE_OPENGL_LEGACY=1 DISABLE_SKYBOX=1 DISABLE_AUDIO=1`

### Other

Consult the [Dockerfile](./Dockerfile) for required libraries and steps.

### Notes:

For *best* performance:
 - Use `ENABLE_OPENGL_LEGACY=1` to enable the legacy OpenGL renderer
 - Use `DISABLE_SKYBOX=1` to avoid drawing the skybox
 - Use `DISABLE_AUDIO=1` to save your ears and some CPU cycles
 - Use `DOS_GL=dmesa` to enable 3Dfx-backed OpenGL instead of software-backed (this only works with legacy OpenGL!)

### 3Dfx mode:

When `DOS_GL` is set to `dmesa`, the game will render using FXMesa, which is a Mesa driver that uses 3Dfx for rendering.
That means you will need a 3Dfx card (Voodoo II and above recommended, but will run on a Voodoo I probably) and appropriate
drivers to run it.

The drivers can be obtained from this repository (see [lib/glide3/README.md](lib/glide3/README.md)). Put the appropriate
`glide3x.dxe` file next to the EXE.

When running Windows 9x, you will have to reboot into DOS mode before running the game for this to work.

dosbox-x and PCem can emulate Voodoo cards for testing purposes.

## Running

1. You'll need a copy of [CWSDPMI.EXE](https://sandmann.dotster.com/cwsdpmi/) alongside the executable.
2. Either `cp build/us_dos/sm64.us.f3dex2e.exe .` or symlink `ln -s build/us_dos/sm64.us.f3dex2e.exe sm64.exe`
3. Run with `dosbox sm64.exe` or try on some real hardware.

## Controls

Controls can be editted as per the PC port, tweak the `SM64CONF.TXT` which is created with defaults upon first run.

## Project Structure

```
sm64
├── actors: object behaviors, geo layout, and display lists
├── asm: handwritten assembly code, rom header
│   └── non_matchings: asm for non-matching sections
├── assets: animation and demo data
│   ├── anims: animation data
│   └── demos: demo data
├── bin: C files for ordering display lists and textures
├── build: output directory
├── data: behavior scripts, misc. data
├── doxygen: documentation infrastructure
├── enhancements: example source modifications
├── include: header files
├── levels: level scripts, geo layout, and display lists
├── lib: SDK library code
├── rsp: audio and Fast3D RSP assembly code
├── sound: sequences, sound samples, and sound banks
├── src: C source code for game
│   ├── audio: audio code
│   ├── buffers: stacks, heaps, and task buffers
│   ├── engine: script processing engines and utils
│   ├── game: behaviors and rest of game source
│   ├── goddard: Mario intro screen
│   ├── menu: title screen and file, act, and debug level selection menus
│   └── pc: port code, audio and video renderer
├── text: dialog, level names, act names
├── textures: skybox and generic texture data
└── tools: build tools
```

## Credits

 - [fgsfdsfgs](https://github.com/fgsfdsfgs); audio, frameskip, dithering, more++

## Contributing

Pull requests are welcome. For major changes, please open an issue first to
discuss what you would like to change.

Official Discord: https://discord.gg/7bcNTPK
