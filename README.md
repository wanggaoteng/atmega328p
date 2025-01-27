# atmega328p
![image](screenshot.jpg)

A very lighted development tool for `atmega328p`, the flasher is [dwdebug](https://github.com/dcwbrown/dwire-debug).   
**Any improment will be greatly appreciate.**
	
## How to install
  * Install `AVR-Toolchain.exe`, after installed, a new variable, AVR_TOOLCHAIN, with its value which provided by your installation path, will be added to the Environment variable. Under the installation path a folder named **bin** contains the toolchains for atmega328p, such as avr-gcc.exe, make.exe, dwdebug.exe. 
  * Install two extensions in vscode: `Make support and task provider`, published by carlos-algms, `Serial Monitor`, published by Microsoft.

## How to use
Open the demo folder by vscode. At the left lower corner of MAKEFILE TASKS:
  * build: used for compiling, and generates .elf, .bin, etc, files in the output folder.
  * flash: the .elf file in output folder will be flashed to atmega328p.
  * debug: debug the program in assambly level. Refer to [dwdebug](https://github.com/dcwbrown/dwire-debug) for more information.
  * config: get the fuses.
  * clean: delete all files in the output folder.
  * reset: software reset of atmega328p.
  
## Auxiliary files in demo
DO NOT DELETE/MODIFY ANY OF THEM, otherwise the dwdebug.exe will not get the COM number automatically.
  * serial.txt: store the outputs which produced by dwdebug.
  * serial.bat: called by Makefile, it will analysis the serial.txt, and returns the COM number which used for dwdebug.
	
## Note
  * As the usb-uart converter chip is CH342K, please install the [chip driver](https://www.wch-ic.com/products/CH342.html?) to your PC.
	