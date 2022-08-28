# Hyper BandCHIP

Next generation CHIP-8 emulator that is designed to take advantage of a new extension built from different foundations.  This includes the original CHIP-8, SuperCHIP V1.1, and XO-CHIP instruction sets.  You can also run different supported cores such as the original CHIP-8, SuperCHIP, XO-CHIP and HyperCHIP-64.

As of right now, only the OpenGL 2.1, OpenGL 3.0, OpenGL ES 2.0 and OpenGL ES 3.0 renderers are built.  A Direct3D 11 (targeting Direct3D 10/Shader Model 4.0) renderer should be built in the near future.

## Requirements for Compiling

- [CMake](https://www.cmake.org/download/) (at least 3.10)
- [GLEW](http://glew.sourceforge.net) (If you're compiling with OpenGL 2.1 or OpenGL 3.0 renderer support)
- [SDL2](https://www.libsdl.org/download-2.0.php) (Latest stable development versions should work fine)
- [libmsbtfont](https://github.com/Bandock/libmsbtfont) (Minimum is 0.2.1, though should compile with/use the latest build)
- C++ Compiler with C++20 Support (mainly for Concepts and some other features)

## How to use

To get started with this current build of the emulator, simply go to the Configuration menu and change the Core (using left/right arrow keys or even return) to either CHIP-8, SuperCHIP, XO-CHIP, or HyperCHIP-64.  Next is to alter the behaviors through the Behaviors menu (which varies depending on the core selected).  Might want to toy with different behaviors due to games that take advantage of these quirks.

Next to thing to do is return to the main menu and select `Load Program` or `Load CHIP-8 Binary Program`.  Make sure when you select the file that it is a valid CHIP-8 program.  Go ahead and select it, let it load.  If successful, you should be able to run it now.  Be aware that programs may error out during execution.

When loading CHIP-8 Binary Program files, some of the options work a bit differently.  Behavior control can affect the platform used (such as at least two CHIP-48 behaviors in the CHIP-8 core forces the CHIP-48 platform instead).  Some games have a chance of setting their own settings (overriding the ones found in the Configuration menu).

That should get you started.  This little guide is likely to change as new features get added.
