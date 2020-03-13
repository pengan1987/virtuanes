# VirtuaNES for BBK Floppy Drive Model I

## What is it?

This emulator is forked from VirtuaNES for emulating BBK Floppy Drive Model I, which is a NES based educational computer available around 1997 in China.

This model with a build in keyboard, IBM compatible 3.5 inch 1.44MB floppy drive, 512KB RAM and 32KB VRAM, The build-in BIOS and DOS(BBGDOS) in MS-DOS FAT compatible.

A Chinese developer fanoble did this work around 2016, but never released outside Chinese community. I made this repository for someone want port the BBK support to other actively developing emulators, e.g. MAME or FCEUX.

## How to build it?

I have successfully built this code with:

Visual C++ 6.0 SP6  
DirectX 7.0 SDK  
Windows XP SP3

The build procedure is quite straightforward, after you add LIB and INCLUDE path of DirectX 7.0 SDK to VC++ 6.0 IDE, you can build and run it.

NOTE: When you build the project, you need to set the active project configuration to "Win32 Release" or "Win32 Debug" otherwise the change for BBK emulation is not compiled

## How it works?

After you build it, you need a modified NES ROM file with first 4 bytes "BBKE" as a "key" file, when the emulator detected the "BBKE" header, it will not run this ROM and load the "ROM.BIN" provided with this repository, this is the BIOS ROM dumped from the BBK machine.

After the "ROM.BIN" loaded, it will automatically load "000.img" it's a standard MS-DOS format floppy disk image which contains BBKDOS and serval applications, since this emulator is still in very early stage, not all applications runs but basically you can see how a BBK machine works.
