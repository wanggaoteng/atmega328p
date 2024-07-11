#build: make
#flash: make flash
#debug: make debug
#reset: make reset
#clean: make clean
#config: make config

#User defined
MCU_TARGET     = atmega328p
CLK			   = 8000000UL
UPLOADER       = dwdebug
PRG            = main
DFP = ${AVR_TOOLCHAIN}/ATmega_DFP


# You should not have to change anything below here, except flash and debug.
SRC            = $(wildcard *.c)
OBJ            = $(patsubst %.c, output/%.o, $(SRC))

OPTIMIZE       = -Os

DEFS           =
LIBS           =

CC             = avr-gcc
SIZE           = avr-objdump -Pmem-usage


# Override is only needed by avr-lib build system.
# Add --param=min-pagesize=0 if warning: array subscript 0 is outside array bounds of 'volatile uint8_t[0]'  
override CFLAGS        = -Wall -ffreestanding -DF_CPU=$(CLK) $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) -B $(DFP)/gcc/dev/$(MCU_TARGET) -I $(DFP)/include
override LDFLAGS       = -Wl,-Map,output/$(PRG).map

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

build: clean all

all: output/$(PRG).elf lst text eeprom
	@$(SIZE) output/$(PRG).elf

output/$(PRG).elf: $(OBJ)
	@$(CC) $(LDFLAGS) -g $(CFLAGS) -o $@ $^ $(LIBS)
	
output/%.o: %.c
	@$(CC) -g $(CFLAGS) -c $< -o $@
	
lst:  output/$(PRG).lst

output/%.lst: output/%.elf
	@$(OBJDUMP) -h -S $< > $@
	
# Rules for building the .text rom images

text: hex bin srec

hex:  output/$(PRG).hex
bin:  output/$(PRG).bin
srec: output/$(PRG).srec

output/%.hex: output/%.elf
	@$(OBJCOPY) -j .text -j .data -O ihex $< $@

output/%.srec: output/%.elf
	@$(OBJCOPY) -j .text -j .data -O srec $< $@

output/%.bin: output/%.elf
	@$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  output/$(PRG)_eeprom.hex
ebin:  output/$(PRG)_eeprom.bin
esrec: output/$(PRG)_eeprom.srec

output/%_eeprom.hex: output/%.elf
	@$(OBJCOPY) -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex $< $@

output/%_eeprom.srec: output/%.elf
	@$(OBJCOPY) -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O srec $< $@

output/%_eeprom.bin: output/%.elf
	@$(OBJCOPY) -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O binary $< $@
	
# Every thing below here is used by avr-libc's build system and can be ignored
# by the casual user.

FIG2DEV                 = fig2dev
EXTRA_CLEAN_FILES       = *.hex *.bin *.srec

dox: eps png pdf

eps: $(PRG).eps
png: $(PRG).png
pdf: $(PRG).pdf

%.eps: %.fig
	$(FIG2DEV) -L eps $< $@

%.pdf: %.fig
	$(FIG2DEV) -L pdf $< $@

%.png: %.fig
	$(FIG2DEV) -L png $< $@
	

clean:
	@del /q output\*.*

flash:  
	@$(UPLOADER) device com$(shell serial.bat),l output/$(PRG).elf,reset,qr
	
debug:
	@$(UPLOADER) device com$(shell serial.bat),l output/$(PRG).elf

reset:
	@$(UPLOADER) device com$(shell serial.bat),reset,qr 

config:
	@$(UPLOADER) device com$(shell serial.bat),config,qr 

