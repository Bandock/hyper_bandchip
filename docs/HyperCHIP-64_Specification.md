# HyperCHIP-64 Extension

This extension builds upon the foundations of CHIP-8, SuperCHIP (V1.0/V1.1), and XO-CHIP (By John Earnest).  It also has some support for instructions from CHIP-8E.

Main purpose behind this extension is to not only enable full access to the 16-bit memory range, but also grant capabilities reminiscent to early computers and consoles 
of the 1980s (compared to the 1970s found when CHIP-8 first existed).  That includes the ability to utilize 4KB of external storage (mimicking battery-backed storage on game
cartridges; support which is dependent on the interpreter that implements it) for each program.  It will also have the ability to utilize XO-CHIP's sample-based audio, but
upgraded to utilize two channels (Stereo).

Technical Specifications:
- Standard CPU Clock:  Maxed at 12 million cycles per second (Variable)
- Endianness:  Big (Same as the original CHIP-8)
- RAM:  64KB (First 512 bytes still reserved)
- 16 Color Support (Through bit planes)
- Stereo Audio with 4 Independent Voices

Here are the supported instructions below:

|Instruction |Description |Extension Inherited From |HyperCHIP-64 Modified|
|------------|------------|-------------------------|---------------------|
|00CN|Scroll Display N Pixels Down|SuperCHIP V1.1|No|
|00DN|Scroll Display N Pixels Up|XO-CHIP|No|
|00E0|Clear Screen|CHIP-8|No|
|00EE|Return Subroutine|CHIP-8|No|
|00FB|Scroll Right 4 Pixels|SuperCHIP V1.1|No|
|00FC|Scroll Left 4 Pixels|SuperCHIP V1.1|No|
|00FD|Exit Interpreter|SuperCHIP V1.0|No|
|00FE|Disable High Resolution Mode|SuperCHIP V1.0|No|
|00FF|Enable High Resolution Mode|SuperCHIP V1.0|No|
|1NNN|Jump to Address at NNN|CHIP-8|No|
|2NNN|Call Subroutine at Address NNN|CHIP-8|No|
|3XNN|Skip the Following Instruction If VX == NN|CHIP-8|No|
|4XNN|Skip the Following Instruction If VX != NN|CHIP-8|No|
|5XY0|Skip the Following Instruction If VX == VY|CHIP-8|No|
|5XY2|Store VX to VY in memory starting at I (Does not increment I)|CHIP-8E|No|
|5XY3|Load VX to VY from memory starting at I (Does not increment I)|CHIP-8E|No|
|6XNN|Set VX to NN|CHIP-8|No|
|7XNN|Add NN to VX|CHIP-8|No|
|8XY0|Set VX to VY|CHIP-8|No|
|8XY1|Set VX to VX OR VY|CHIP-8|No|
|8XY2|Set VX to VX AND VY|CHIP-8|No|
|8XY3|Set VX to VX XOR VY|CHIP-8|No|
|8XY4|Add VY to VX (VF = 01 for Carry, 00 for No Carry)|CHIP-8|No|
|8XY5|Subtract VY from VX (VF = 00 for Borrow, 01 for No Borrow)|CHIP-8|No|
|8XY6|Store VY shifted one bit to the right in VX (VF = LSB)|CHIP-8|No|
|8XY7|Set VX to VY - VX (VF = 00 for Borrow, 01 for No Borrow)|CHIP-8|No|
|8XY8|Store VY rotated one bit to the right in VX|HyperCHIP-64|N/A|
|8XY9|Store VY rotated one bit to the left in VX|HyperCHIP-64|N/A|
|8XYA|Test VX AND VY (VF = 00 for Zero Result, 01 for Non-Zero Result)|HyperCHIP-64|N/A|
|8XYB|Set VX to NOT VY|HyperCHIP-64|N/A|
|8XYE|Store VY shifted one bit to the left in VX (VF = MSB)|CHIP-8|No|
|9XY0|Skip the Following Instruction If VX != VY|CHIP-8|No|
|ANNN|Set I to NNN|CHIP-8|No|
|BNNN|Jump to Address at NNN + V0|CHIP-8|No|
|CXNN|Set VX to Random Number (Mask = NN)|CHIP-8|No|
|DXYN|Draw Sprite at VX, VY (If N == 0, then draw a 16x16 sprite) (VF = 01 if pixels were unset, 00 if no pixels were unset)|CHIP-8/SuperCHIP V1.0|No|
|EX9E|Skip the Following Instruction If Hex Key Pressed == VX|CHIP-8|No|
|EXA1|Skip the Following Instruction If Hex Key Not Pressed == VX|CHIP-8|No|
|F000 NNNN|Set I to NNNN|XO-CHIP|No|
|FN01|Sets the current drawing bit plane (N = 0 for No Draw, N = 1 for Plane 1, N = 2 for Plane 2, N = 3 for Plane 1 and 2)|XO-CHIP|No|
|F002|Load the audio buffer from memory at I.|XO-CHIP|Yes, upgraded to support multiple voices (up to 4 at the moment).  Loads the data into the selected voice's audio buffer.|
|FX07|Store Delay Timer to VX|CHIP-8|No|
|FX0A|Wait for Keypress and Store in VX|CHIP-8|No|
|FX15|Set Delay Timer to VX|CHIP-8|No|
|FX18|Set Sound Timer to VX|CHIP-8|Yes, can modify another sound timer based on the currently selected voice.|
|FX1E|Add Value Stored in VX to I|CHIP-8|No|
|FX20|Indirect Jump to Address stored in I + VX (Big Endian)|HyperCHIP-64|N/A|
|FX21|Indirect Call Subroutine at Address stored in I + VX (Big Endian)|HyperCHIP-64|N/A|
|FX29|Point I to 5-byte font sprite for digit in VX (0-F)|CHIP-8|No|
|FX30|Point I to 10-byte font sprite for digit in VX (0-F)|SuperCHIP V1.1|No|
|FX33|Store BCD in VX at I, I+1, and I+2|CHIP-8|No|
|FX3A|Sets the pitch to the value stored in VX|XO-CHIP|Yes, can modify the pitch of the selected voice.|
|FX3B|Sets the volume of the selected voice to the value stored in VX.|HyperCHIP-64|N/A|
|FN3C|Sets the current voice specified by N for audio operations (Currently maxed at 4 voices, ranging from 0 to 3.)|HyperCHIP-64|N/A|
|FN3D|Sets the audio channel mask for the selected voice (N = 0 for No Audio Output, N = 1 for Channel 1 (Left), N = 2 for Channel 2 (Right))|HyperCHIP-64|N/A|
|FX55|Store V0 to VX in memory starting at I (I = I + X + 1, CHIP-8 original behavior)|CHIP-8|No|
|FX65|Load V0 to VX from memory starting at I (I = I + X + 1, CHIP-8 original behavior)|CHIP-8|No|
|FX75|Store V0 to VX in RPL User Flags (X <= 15)|SuperCHIP V1.0, XO-CHIP 1.1|No|
|FX85|Load V0 to VX from RPL User Flags (X <= 15)|SuperCHIP V1.0, XO-CHIP 1.1|No|
|FXA2|Set I to the address stored in I + VX (Big Endian)|HyperCHIP-64|N/A|

Currently Work-In-Progress.
