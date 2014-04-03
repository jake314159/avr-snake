
all: avrSnake.hex size
install: transfer size

SHELL = /bin/sh

FILE = avrSnake
LIBS = RIOS.c

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
SIZE    = avr-size

CC_FLAGS = -Wall -Os
CHIP_COMPILE = atmega644p
CHIP_FLASH = m644p
DF_CPU = 12000000

$(FILE).elf: $(FILE).c lcdlib/liblcd.a
	$(CC) -mmcu=$(CHIP_COMPILE) -DF_CPU=$(DF_CPU) $(CC_FLAGS) $(FILE).c $(LIBS) lcdlib/liblcd.a -o $(FILE).elf

lcdlib/liblcd.a: 
	$(MAKE) -C lcdlib/make

$(FILE).hex: $(FILE).elf
	$(OBJCOPY) -O ihex $(FILE).elf $(FILE).hex

# Display size of file.
size: $(FILE).elf
	@echo
	$(SIZE) -C --mcu=$(CHIP_COMPILE) $(FILE).elf

transfer: avrSnake.hex
	$(AVRDUDE) -c usbasp -p $(CHIP_FLASH) -U flash:w:$(FILE).hex

clean:
	rm -f *.hex
	rm -f *.elf
