# Hyper BandCHIP

Next generation CHIP-8 emulator that is designed to take advantage of a new extension built from different foundations.
This includes the original CHIP-8, SuperCHIP V1.1, and XO-CHIP instruction sets.  You can also run different supported cores
such as the original CHIP-8, SuperCHIP, XO-CHIP and HyperCHIP-64.

As of right now, only the OpenGL 2.1, OpenGL ES 2.0 and OpenGL ES 3.0 renderers are built.  OpenGL 3.3 and Direct3D renderers should be built
in the near future.

## Requirements for Compiling

- [CMake](https://www.cmake.org/download/) (at least 3.10)
- [GLEW](http://glew.sourceforge.net) (If you're compiling with OpenGL 2.1, OpenGL ES 2.0, or OpenGL ES 3.0 renderer support)
- [SDL2](https://www.libsdl.org/download-2.0.php) (Latest stable development versions should work fine)
- C++ Compiler with C++20 Support (mainly for Concepts and some other features)

## How to use

To get started with this current build of the emulator, simply go to the Configuration menu and change the Core (using left/right 
arrow keys or even return) to either CHIP-8, SuperCHIP, XO-CHIP, or HyperCHIP-64.  Next is to alter the behaviors through the 
Behaviors menu (which varies depending on the core selected).  Might want to toy with different behaviors due to games that take 
advantage of these quirks.

Next to thing to do is return to the main menu and select Load Program.  White items in that menu are directories, yellow items 
are files, and red indicates the current selected item.  Make sure when you select the file that it is a valid CHIP-8 program.
Go ahead and select it, let it load.  If successful, you should be able to run it now.  Be aware that programs may error out
during execution.  Error messages will be implemented in the near future.

That should get you started.  This little guide is likely to change as new features get added.
