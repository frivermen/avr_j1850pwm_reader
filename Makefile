# compiler options
CC=./avr-gcc-8.3.0-x64-linux/bin/avr-
CFLAGS= -Wall -Os -mmcu=atmega328p –param=min-pagesize=0

all:
	$(CC)gcc -Wall -Os -mmcu=atmega328p main.c -o main.o
	$(CC)objcopy -j .text -j .data -O ihex main.o main.hex
	$(CC)size main.o

# arduino 16MHz
optiboot: all
	avrdude -c arduino -P /dev/ttyUSB0 -b115200 -p atmega328p -U flash:w:main.hex:i

# arduino 8MHz
optiboot-8mhz: all
	avrdude -c arduino -P /dev/ttyUSB0 -b57600 -p atmega328p -U flash:w:main.hex:i

usbasp: all
	avrdude -c usbasp -p atmega328p -U flash:w:main.hex:i

clean:
	rm -f main.o
