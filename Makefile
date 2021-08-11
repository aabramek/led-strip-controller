CC = avr-gcc.exe
AR = avr-ar.exe
DEVICE = atmega8a
DEVICESHORT = m8
FCPU = F_CPU=1000000
CFLAGS = -Wall -Iinclude -IC:/users/admin/programowanie/programy/c/avr/usart/include -O2 -mmcu=$(DEVICE) -D$(FCPU)
PROGRAMMER = usbasp
OBJ = obj/main.o
LIB = C:/users/admin/programowanie/programy/c/avr/usart/bin/libusart.a

.PHONY: upload, all

all: bin/kontroler.elf

bin/kontroler.elf: $(OBJ) $(LIB)
	$(CC) $(CFLAGS) $(OBJ) $(LIB) -o bin/kontroler.elf

obj/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

bin/test.elf: obj/test.o
	$(CC) $(CFLAGS) obj/test.o $(LIB) -o bin/test.elf

obj/test.o: src/test.c
	$(CC) $(CFLAGS) -c src/test.c -o obj/test.o

upload:
	avrdude.exe -c $(PROGRAMMER) -p $(DEVICESHORT) -U flash:w:bin/kontroler.elf