# HyperCHIP-64 Extension

This extension builds upon the foundations of CHIP-8 and SuperCHIP (V1.0/V1.1).  It also has some support for instructions from CHIP-8E and XO-CHIP by John Earnest.

Main purpose behind this extension is to not only enable full access to the 16-bit memory range, but also grant capabilities reminiscent to early computers and consoles 
of the 1980s (compared to the 1970s found when CHIP-8 first existed).  That includes the ability to utilize 4KB of external storage (mimicking battery-backed storage on game
cartridges) for each program and 4 working programmable sound generators (nearly equivalent to the Commodore 64's SID).

Technical Specifications:
- Standard CPU Clock:  1,800 cycles per second
- RAM:  64KB

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
|1NNN|Jump to Address at NNN|CHIP-8|Yes, can be extended to 16-bits by the 4-bit Absolute Address Extend Prefix.|
|2NNN|Call Subroutine at Address NNN|CHIP-8|Yes, can be extended to 16-bit by the 4-bit Absolute Address Extend Prefix.|
|3XNN|Skip the Following Instruction If VX == NN|CHIP-8|Yes, can skip two or more instructions if prefixes are found.|
|4XNN|Skip the Following Instruction If VX != NN|CHIP-8|Yes, can skip two or more instructions if prefixes are found.|
|5XY0|Skip the Following Instruction If VX == VY|CHIP-8|Yes, can skip two or more instructions if prefixes are found.|
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
|9XY0|Skip the Following Instruction If VX != VY|CHIP-8|Yes, can skip two or more instructions if prefixes are found.|
|ANNN|Set I to NNN|CHIP-8|Yes, can be extended to 16-bit by the 4-bit Absolute Address Extend Prefix.|
|BNNN|Jump to Address at NNN + V0|CHIP-8|Yes, can be extended to 16-bit by the 4-bit Absolute Address Extend Prefix.  Can also override the V0 register by the V Register Offset Override Prefix.|
|CXNN|Set VX to Random Number (Mask = NN)|CHIP-8|No|
|DXYN|Draw Sprite at VX, VY (If N == 0, then draw a 16x16 sprite)|CHIP-8/SuperCHIP V1.0|No|
|EX9E|Skip the Following Instruction If Hex Key Pressed == VX|CHIP-8|Yes, can skip two or more instructions if prefixes are found.|
|EXA1|Skip the Following Instruction If Hex Key Not Pressed == VX|CHIP-8|Yes, can skip two or more instructions if prefixes are found.|
|FX07|Store Delay Timer to VX|CHIP-8|No|
|FX0A|Wait for Keypress and Store in VX|CHIP-8|No|
|FX15|Set Delay Timer to VX|CHIP-8|No|
|FX18|Set Sound Timer to VX|CHIP-8|No|
|FX1E|Add Value Stored in VX to I|CHIP-8|No|
|FX20|Indirect Jump to Address stored in I + VX|HyperCHIP-64|N/A|
|FX21|Indirect Call Subroutine at Address stored in I + VX|HyperCHIP-64|N/A|
|FX29|Point I to 5-byte font sprite for digit in VX (0-F)|CHIP-8|No|
|FX30|Point I to 10-byte font sprite for digit in VX (0-9)|SuperCHIP V1.1|No|
|FX33|Store BCD in VX at I, I+1, and I+2|CHIP-8|No|
|FX55|Store V0 to VX in memory starting at I (I = I + X + 1, CHIP-8 original behavior)|CHIP-8|No|
|FX65|Load V0 to VX from memory starting at I (I = I + X + 1, CHIP-8 original behavior)|CHIP-8|No|
|FX75|Store V0 to VX in RPL User Flags (X <= 7)|SuperCHIP V1.0|No|
|FX85|Load V0 to VX from RPL User Flags (X <= 7)|SuperCHIP V1.0|No|
|FXA0|Latch To Memory Instruction *WIP*|HyperCHIP-64|N/A|
|FXA1|Unlatch From Memory Instruction *WIP*|HyperCHIP-64|N/A|
|FXA2|Set I to [I + VX]|HyperCHIP-64|N/A|
|FNB0|4-bit Absolute Address Extend Prefix (N is stored in the last nibble of the 16-bit address when used on a supported instruction.)|HyperCHIP-64|N/A|
|FXB1|V Register Offset Override Prefix (Replaces the default register and instead uses another register as the offset.)|HyperCHIP-64|N/A|

Currently Work-In-Progress.
