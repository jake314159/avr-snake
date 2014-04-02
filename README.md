AVR-snake
===============

Compiling and installing 
------------------------

After downloading the source code you can use the included make file to compile and install the program onto your micro-controller.

To compile the tool use the command

```bash
make
```

Or you can install and flash the program to your chip using avrdude with the command. This may require root access

```bash
make install
```

Compiling and installing has been tested on *Ubuntu 14.04* with the *atmega644p* chip. Other platforms or chips may require some changes to the make file or source code.
