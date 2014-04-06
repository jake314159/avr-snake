
SHELL = /bin/sh

BIN = bin/
SRC = src/

FILE = avrSnake
LIBS = $(SRC)RIOS.c

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
SIZE    = avr-size

# To debug add -DDEBUG to CC_FLAGS
CC_FLAGS = -Wall
CHIP_COMPILE = atmega644p
CHIP_FLASH = m644p
DF_CPU = 12000000

.PHONY: all
all: $(BIN)avrSnake.hex 
#size
.PHONY: install
install: transfer 
#size

$(BIN)$(FILE).elf: $(SRC)$(FILE).c lcdlib/liblcd.a $(SRC)lcd.h $(BIN)
	$(CC) -mmcu=$(CHIP_COMPILE) -DF_CPU=$(DF_CPU) -Os $(CC_FLAGS) $(SRC)$(FILE).c $(LIBS) lcdlib/liblcd.a -o $(BIN)$(FILE).elf

lcdlib/liblcd.a: 
	$(MAKE) -C lcdlib/make

$(BIN)$(FILE).hex: $(BIN)$(FILE).elf
	$(OBJCOPY) -O ihex $(BIN)$(FILE).elf $(BIN)$(FILE).hex

$(BIN): 
	mkdir $(BIN)

$(SRC)lcd.h: lcdlib/lcd.h
	cp lcdlib/lcd.h $(SRC)/lcd.h

# Display size of file.
.PHONY: size
size: $(BIN)$(FILE).elf
	@echo
	$(SIZE) -C --mcu=$(CHIP_COMPILE) $(BIN)$(FILE).elf

.PHONY: transfer
transfer: $(BIN)avrSnake.hex
	$(AVRDUDE) -c usbasp -p $(CHIP_FLASH) -U flash:w:$(BIN)$(FILE).hex

.PHONY: clean
clean:
	rm -f $(BIN)*.hex
	rm -f $(BIN)*.elf
