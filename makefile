
SHELL = /bin/sh

FILE = avrSnake
LIBS = src/RIOS.c

BIN = bin/
SRC = src/

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
SIZE    = avr-size

CC_FLAGS = -Wall -Os
CHIP_COMPILE = atmega644p
CHIP_FLASH = m644p
DF_CPU = 12000000

all: $(BIN)avrSnake.hex size
install: transfer size

$(BIN)$(FILE).elf: $(SRC)$(FILE).c lcdlib/liblcd.a $(SRC)lcd.h $(BIN)
	$(CC) -mmcu=$(CHIP_COMPILE) -DF_CPU=$(DF_CPU) $(CC_FLAGS) $(SRC)$(FILE).c $(LIBS) lcdlib/liblcd.a -o $(BIN)$(FILE).elf

lcdlib/liblcd.a: 
	$(MAKE) -C lcdlib/make

$(BIN)$(FILE).hex: $(BIN)$(FILE).elf
	$(OBJCOPY) -O ihex $(BIN)$(FILE).elf $(BIN)$(FILE).hex

$(BIN): 
	mkdir $(BIN)

$(SRC)lcd.h: lcdlib/lcd.h
	cp lcdlib/lcd.h $(SRC)/lcd.h

# Display size of file.
size: $(BIN)$(FILE).elf
	@echo
	$(SIZE) -C --mcu=$(CHIP_COMPILE) $(BIN)$(FILE).elf

transfer: $(BIN)avrSnake.hex
	$(AVRDUDE) -c usbasp -p $(CHIP_FLASH) -U flash:w:$(BIN)$(FILE).hex

clean:
	rm -f $(BIN)*.hex
	rm -f $(BIN)*.elf
