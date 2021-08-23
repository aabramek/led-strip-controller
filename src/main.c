#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include <util/atomic.h>

// micro seconds passed since timer start call
volatile unsigned int counter = 0;

uint8_t InputState(void);

void TimerStart(void);

unsigned int TimerStop(void);

uint8_t ReadNEC(unsigned int* address, unsigned int* command);

unsigned int ReadNECBit(void);

void SaveConfigToEEPROM(void);

void LoadConfigFromEEPROM(void);

int main(void)
{
	sei();
	TIMSK = (1 << TOIE0);
	OCR1AH = OCR1BH = 0;
	/*OCR1AL = 128;
	OCR1BL = 0;
	OCR2 = 0;*/
	LoadConfigFromEEPROM();
	DDRB = (1 << DDRB1) | (1 << DDRB2) | (1 << DDRB3);
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
	TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << CS22);
	unsigned int address = 0;
	unsigned int command = 0;
	enum Color { red, green, blue } color, color_current;
	color = color_current = red;
	uint8_t value = 0;
	while (1)
	{
		uint8_t status = ReadNEC(&address, &command);
		if (status != 0)
		{
			// Error or repeated code
			continue;
		}
		switch (command)
		{
			case 0xA25D: color = red; break;
			case 0x629D: color = green; break;
			case 0xE21D: color = blue; break;
		}
		if (color != color_current)
		{
			color_current = color;
			continue;
		}
		switch (command)
		{
			case 0x6897: value = 0; break;
			case 0x30CF: value = 25; break;
			case 0x18E7: value = 51; break;
			case 0x7A85: value = 76; break;
			case 0x10EF: value = 102; break;
			case 0x38C7: value = 127; break;
			case 0x5AA5: value = 153; break;
			case 0x42BD: value = 188; break;
			case 0x4AB5: value = 224; break;
			case 0x52AD: value = 255; break;
			case 0x22DD:
				LoadConfigFromEEPROM();
			break;
			case 0x02FD:
				SaveConfigToEEPROM();
			break;
		}
		switch (color)
		{
			case red: OCR1AL = value; break;
			case green: OCR1BL = value; break;
			case blue: OCR2 = value; break;
		}
	}
}

uint8_t InputState(void)
{
	return PINB & (1 << PINB0) ? 1 : 0;
}

void TimerStart(void)
{
	TCNT0 = 0;
	TCCR0 = (1 << CS00);
}

unsigned int TimerStop(void)
{
	TCCR0 = 0;
	unsigned int c = counter;
	counter = 0;
	return c + (unsigned int)TCNT0;
}

uint8_t ReadNEC(unsigned int* address, unsigned int* command)
{
	unsigned int time = 0;

	//Wait for low state
	while (InputState() == 1);
	TimerStart();
	
	//Wait for high state again, if it took about 9ms its initial burst
	while (InputState() == 0);
	time = TimerStop();
	
	if (time < 8700 || time > 9300)
		return 1;
	
	//Initial burst is followed by 4.5ms space or 2.25ms for repeated code
	TimerStart();
	while (InputState() == 1);
	time = TimerStop();

	if (time > 2000 && time < 2400)
	{
		while (InputState() == 0);
		return 2;
	}

    unsigned int addr = 0;
	unsigned int cmd = 0;

	//Read 16 bits of address
	for (uint8_t i = 0; i < 16; ++i)
	{
		addr <<= 1;
		addr |= ReadNECBit();
	}

	//Read 16 bits of command
	for (uint8_t i = 0; i < 16; ++i)
	{
		cmd <<= 1;
		cmd |= ReadNECBit();
	}

	//Final 0.56ms burst signifies end of transmission
	while (InputState() == 0);

	*address = addr;
	*command = cmd;

	return 0;
}

unsigned int ReadNECBit(void)
{
	unsigned int time = 0;
	while (InputState() == 0);
	TimerStart();
	while (InputState() == 1);
	time = TimerStop();
	return time > 1300 ? 1 : 0;
}

void SaveConfigToEEPROM(void)
{
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t*)1, OCR1AL);
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t*)2, OCR1BL);
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t*)3, OCR2);
}

void LoadConfigFromEEPROM(void)
{
	eeprom_busy_wait();
	OCR1AL = eeprom_read_byte((uint8_t*)1);
	OCR1BL = eeprom_read_byte((uint8_t*)2);
	OCR2 = eeprom_read_byte((uint8_t*)3);
}

ISR(TIMER0_OVF_vect)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		counter += 256;
	}
}
