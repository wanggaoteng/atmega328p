# atmega328p
![image](screenshot.jpg)

A very lighted development tool for `atmega328p`, the flasher is [dwdebug](https://github.com/dcwbrown/dwire-debug)
	
## How to install
  * Install `AVR-Toolchain.exe`, after installed, a Environment variable will be added to the Path, such as `x:\software\AVR-Toolchain\bin`, this bin folder contains the toolchains for atmega328p, such as avr-gcc.exe, make.exe, dwdebug.exe. 
  * Install two extensions in vscode: `Make support and task provider`, published by carlos-algms, `Serial Monitor`, published by Microsoft.

## How to use
Open the demo folder by vscode. At the left lower corner of MAKEFILE TASKS:
  * build: used for compiling, and generates .elf, .bin, etc, files in the output folder.
  * flash: the .elf in output folder will be flashed to atmega328p.
  * debug: debug the program in assambly level. Refer to [dwdebug](https://github.com/dcwbrown/dwire-debug) for more information.
  * config: get the fuses.
  * clean: delete all files in the output folder.
  * reset: software reset of atmega328p.
  
## Other files in the demo
  * serial.txt: store the COM number, which the dwdebug needed.
  * serial.bat: called by Makefile, analysis serial.txt, and get the COM number.
	
	