CC = avr-gcc.exe
AR = avr-ar.exe
DEVICE = atmega8a
DEVICESHORT = m8
FCPU = F_CPU=1000000
CFLAGS = -Wall -Iinclude -O2 -mmcu=$(DEVICE) -D$(FCPU)
PROGRAMMER = usbasp
OBJ = obj/main.o

.PHONY: upload

all: bin/kontroler.elf

bin/kontroler.elf: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIB) -o $@

obj/main.o: src/main.c
	$(CC) $(CFLAGS) -c $^ -o $@

upload:
	avrdude.exe -c $(PROGRAMMER) -p $(DEVICESHORT) -U flash:w:bin/kontroler.elf