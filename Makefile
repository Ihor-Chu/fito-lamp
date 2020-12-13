CFLAGS = -mmcu=atmega8 -Wall -Os -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions -I. -std=c99 -DF_CPU=$(F_CPU)
CC = avr-gcc
OBJECTS =  main.o setup.o adc.o button.o pid.o
F_CPU = 16000000L

all:	
	make hello.hex

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.S.o:
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
.c.s:
	$(CC) $(CFLAGS) -S $< -o $@

clean:
	rm -f *.hex *.o

hello.bin: clean $(OBJECTS)
	#avr-gcc hello.c -o hello.bin $(CFLAGS)
	$(CC) $(CFLAGS) -o hello.bin $(OBJECTS) $(LDFLAGS)

hello.hex: hello.bin
	rm -f hello.hex
	avr-objcopy -j .text -j .data -O ihex hello.bin hello.hex

dude:	hello.hex
#/home/igor/sources/arduino-1.8.8/hardware/tools/avr/bin/avrdude 
# -C/home/igor/.arduino15/packages/MiniCore/hardware/avr/2.0.7/avrdude.conf -v 
# -patmega8 -cstk500v1 -P/dev/ttyACM0 -b19200 -Uflash:w:/tmp/arduino_build_88560/Blink.ino.with_bootloader.hex:i 

	avrdude  -c stk500v1 -p atmega8 -P/dev/ttyACM0 -b19200 -U flash:w:hello.hex -v -v
