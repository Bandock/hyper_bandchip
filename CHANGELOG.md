# Hyper BandCHIP Changelog

## Version 0.14

- Forgot to update the main menu's status of it's changes when a CHIP-8 Binary Program is successfully loaded.  It's fixed now.

- Unlimited Speed is now possible!  To achieve this, just set the cycle rate to `0` and it will set it fast as it could.  Uses a calculation (which maybe tweaked at anytime in the future) to push even higher than the maximum possible setting avalable whenever unlimited speed is not used.

- CHIP-8 Binary Programs can now truly utilize unlimited speed as a result of the upgrade.

- Fixed the OpenGL 3.0 renderer as it was not rendering properly at all.  On top of that, it also failed to compile one of the shaders properly.

## Version 0.13

- Upgraded the XO-CHIP core to support 16 color handling.

- Introducing support for Timendus' CHIP-8 Binary Format (Version 0 right now).  Not meant to replace the current method of loading programs, but instead introduce a way to load programs in a more convenient way.  Properties utilized right now are Desired Execution Speed, Program Name, Program Author (displays the first author currently), Color Configuration (overrides the current palette), and Font Data (made possible thanks to the emulator's existing capabilities).  Platforms supported are CHIP-8, CHIP-48 (possible thanks to the availability of the load/store and shift behaviors; enabled by turning both of those behaviors on in the CHIP-8 core), SuperCHIP V1.0 (Must use the SuperCHIP V1.0 behavior and not the Original SuperCHIP V1.0 behavior), SuperCHIP V1.1 (Must use the Original SuperCHIP V1.1 as Fixed SuperCHIP V1.1 lacks the necessary behaviors for this platform), and XO-CHIP (Can use any behavior in the XO-CHIP core at present currently).

- Readded clipping behavior to the CHIP-8 core as `VIP Clipping`.  This feature was part of the CHIP-8 core in a much earlier build, but was removed.

- Added the `VIP VF Reset` behavior to the CHIP-8 core.

- Modified the behavior of all the load font instructions (in all cores) to mimic the behavior of the original COSMAC VIP CHIP-8 Interpreter.

- Added the `Original SuperCHIP V1.0` behavior to indicate that there was an earlier build/version of that extension where the difference is how you loaded high resolution fonts.  As a result, `SuperCHIP V1.0` behavior now loads high resolution fonts in the same fashion as all the `SuperCHIP V1.1` variants.

- Fixed the Draw Sprite (DXYN) instruction to follow the original behavior of the COSMAC VIP.

- Fixed a bug where if the `VIP Display Interrupt` behavior was enabled on the CHIP-8 core and paused the emulation, returning to it would fail to render until a draw instruction was called.

- Fixed the 'E' character for the menu font.

- Increased the texture size for rendering on the main menu texture for the OpenGL ES 2.0 and 3.0 renderers.  As a result, no more viewport changes are necessary for those renderers.  OpenGL 2.1 renderer still uses the prior system.

- Added a new message for all existing OpenGL-based renderers that lets you know if it failed to create their respective context.  That's also a new fail state to prevent issues from popping up.

- GL Context check is now added to all existing OpenGL-based renderers to prevent issues after the renderer object gets destroyed.

- OpenGL 3.0 Renderer is now available.  This renderer does use a few extensions, which they are currently required.

- When a program is loaded and the machine itself is created, it now displays cycles per frame on the windows title.

## Version 0.12

- Added support for custom font styles through specific uses of the MisbitFont format.  High resolution fonts are stored in `fonts/hires` and low resolution fonts are stored in `fonts/lores`.  Hardcoded fonts are still provided if there are no fonts in either directory.

- Added `VIP Display Interrupt` behavior to the CHIP-8 core, allowing it to mimic the way sprites were displayed on the COSMAC VIP (through the original CHIP-8 interpreter).  Sync feature must be turned on for this to work.
