
all: avrSnake.hex
install: transfer

SHELL = /bin/sh

FILE = avrSnake
LIBS = RIOS.c

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

CC_FLAGS = -Wall -Os
CHIP_COMPILE = atmega644p
CHIP_FLASH = m644p
DF_CPU = 12000000

$(FILE).elf: $(FILE).c lcdlib/liblcd.a
	$(CC) -mmcu=$(CHIP_COMPILE) -DF_CPU=$(DF_CPU) $(CC_FLAGS) $(FILE).c $(LIBS) lcdlib/liblcd.a -o $(FILE).elf

lcdlib/liblcd.a: 
	$(MAKE) -C lcdlib/make

avrSnake.hex: avrSnake.elf
	$(OBJCOPY) -O ihex $(FILE).elf $(FILE).hex

transfer: avrSnake.hex
	$(AVRDUDE) -c usbasp -p $(CHIP_FLASH) -U flash:w:$(FILE).hex

clean:
	rm -f *.hex
	rm -f *.elf
