
/*********************************************************************
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>

#define ACP_DELAY 25

/* global variables */
unsigned char INDEX = 1;   /* 1 to 6 */
unsigned char HOUR = 12;   /* 1 to 12 */
unsigned char MINUTE = 00;  /* 0 to 59 */
unsigned char SECOND = 00;  /* 0 to 59 */

unsigned char serialBuffer[10] = {0,0,0,0,0,0,0,0,0,0};
unsigned char* serialBufferPtr = &serialBuffer[0];
boolean TOP_OF_THE_MINUTE = false;

/* timer1 used for timekeeping */
ISR(TIMER1_COMPA_vect)
{
	/* increment and check seconds */
	if (++SECOND >= 60)
	{
		SECOND = 0;
		TOP_OF_THE_MINUTE = true;

		/* increment and check minutes */
		if (++MINUTE >= 60)
		{
			MINUTE = 0;

			/* increment and check hours */
			if (++HOUR >= 13)
			{
				HOUR = 1;
			}
		}
	}
}

/* timer2 used for nixie tube multiplexing */
ISR(TIMER2_COMPA_vect)
{
	/* HOUR = 10, 11, or 12 or top of minute?*/
	if ((HOUR / 10) | TOP_OF_THE_MINUTE)
	{
		PORTB = 0x10;  /* turn HOUR tens LED on */
	}
	else
	{
		PORTB = 0x00;  /* turn HOUR tens LED off */
	}

	switch(INDEX++)
	{
		/* HOUR tens place */
		case 1:
			/* blank anodes */
			PORTD = 0x00;

			/* set cathode */
			PORTB |= (HOUR / 10);

			/* only turn anode on if one */
			if (HOUR / 10)
			{
				PORTD = 0x04;    
			}
		break;

		/* HOUR ones place */
		case 2:
			/* blank anodes */
			PORTD = 0x00;

			/* set cathode */
			PORTB |= (HOUR % 10);

			/* turn on anode */
			PORTD = 0x08;
		break;

		/* MINUTE tens place */
		case 3:
			/* blank anodes */
			PORTD = 0x00;

			/* set cathode */
			PORTB |= (MINUTE / 10);

			/* turn on anode */
			PORTD = 0x10;
		break;

		/* MINUTE ones place */
		case 4:
			/* blank anodes */
			PORTD = 0x00;

			/* set cathode */
			PORTB |= (MINUTE % 10);

			/* turn on anode */
			PORTD = 0x20;
		break;

		/* SECOND tens place */
		case 5:
			/* blank anodes */
			PORTD = 0x00;

			/* set cathode */
			PORTB |= (SECOND / 10);

			/* turn on anode */
			PORTD = 0x40;
		break;

		/* SECOND ones place */
		case 6:
			/* blank anodes */
			PORTD = 0x00;

			/* set cathode */
			PORTB |= (SECOND % 10);

			/* turn on anode */
			PORTD = 0x80;

			/* reset index */
			INDEX = 1;
		break;
	}
}

void setup()
{
	/* configure pins */
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);
	pinMode(12, OUTPUT);

	/* disable global interrupts */
	cli();

	/* timer1 1Hz interrupt */
	TCCR1A = 0x00;
	TCCR1B = 0x00;
	TCNT1 = 0x0000;
	OCR1A = 0x3D08;
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	TIMSK1 |= (1 << OCIE1A);

	/* timer2 1kHz interrupt */
	TCCR2A = 0x00;
	TCCR2B = 0x00;
	TCNT2 = 0x00;
	OCR2A = 0xF9;
	TCCR2A |= (1 << WGM21);
	TCCR2B |= (1 << CS22);
	TIMSK2 |= (1 << OCIE2A);

	/* enable global interrupts */
	sei();

	Serial.begin(115200);
	Serial.println("    Open Source Nixie Tube Shield v2   ");
	Serial.println("         Switchmode Design, Inc        ");
	Serial.println("Set time using this format: @HHMMSS[CR]");
	Serial.println();
}

void loop()
{
	/* cycle digits at top of minute */
	if (TOP_OF_THE_MINUTE)
	{
		anti_cathode_poisoning();
		TOP_OF_THE_MINUTE = false;
	}

	/* any received serial bytes? */
	if (Serial.available() > 0)
	{
		/* parse serial byte(s) */
		serial_set_time();
	}
}

void anti_cathode_poisoning()
{
	/* save current time */
	unsigned char hour = HOUR;
	unsigned char minute = MINUTE;
	unsigned char second = SECOND;

	/* cycle the digits */
	HOUR = MINUTE = SECOND = 11;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 22;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 77;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 88;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 00;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 66;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 33;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 99;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 44;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 55;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 44;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 99;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 33;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 66;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 00;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 88;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 77;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 22;
	delay(ACP_DELAY);
	HOUR = MINUTE = SECOND = 11;
	delay(ACP_DELAY);

	/* restore current time */
	HOUR = hour;
	MINUTE = minute;
	SECOND = second;
}

void serial_set_time()
{
	/* read a byte from serial port */
	unsigned char rxByte = Serial.read();

	/* first byte needs to be an @ */
	if ((serialBufferPtr == &serialBuffer[0]) && (rxByte == '@'))
	{
		*serialBufferPtr++ = rxByte;
		return;
	}

	/* once we get an @, save bytes in buffer until [CR]*/
	if ((serialBufferPtr != &serialBuffer[0]) && (rxByte != '@'))
	{
		*serialBufferPtr++ = rxByte;
	}

	/* look for a [CR] */
	if (rxByte == 0x0D)
	{ 
		/* make sure values are numbers 0 - 9*/
		if 	(
				serialBuffer[1] >= '0' && serialBuffer[1] <= '9' &&
				serialBuffer[2] >= '0' && serialBuffer[2] <= '9' &&
				serialBuffer[3] >= '0' && serialBuffer[4] <= '9' &&
				serialBuffer[4] >= '0' && serialBuffer[5] <= '9' &&
				serialBuffer[5] >= '0' && serialBuffer[7] <= '9' &&
				serialBuffer[6] >= '0' && serialBuffer[8] <= '9'
			)
		{
			/* set the global time variables */
			HOUR = ((serialBuffer[1] - 0x30) * 10) + (serialBuffer[2] - 0x30);
			MINUTE = ((serialBuffer[3] - 0x30) * 10) + (serialBuffer[4] - 0x30);
			SECOND = ((serialBuffer[5] - 0x30) * 10) + (serialBuffer[6] - 0x30);

			Serial.print("Setting time to ");
			Serial.print(HOUR / 10, DEC);
			Serial.print(HOUR % 10, DEC);
			Serial.print(":");
			Serial.print(MINUTE / 10, DEC);
			Serial.print(MINUTE % 10, DEC);
			Serial.print(":");
			Serial.print(SECOND / 10, DEC);
			Serial.println(SECOND % 10, DEC);
		}
		else
		{
			Serial.println("Bad string format.  Use @HHMMSS[CR]");
		}

		/* clear the buffer */
		for (int i = 0; i < sizeof(serialBuffer); i++)
		{
			serialBuffer[i] = 0x00;
		} 

		/* reset the buffer pointer */ 
		serialBufferPtr = &serialBuffer[0];
	}

	/* make sure we haven't overrun the buffer */
	if (serialBufferPtr == (&serialBuffer[0] + sizeof(serialBuffer)))
	{
		Serial.println("Buffer overrun -- flushing buffer");

		/* throw bytes away */
		while(Serial.available() > 0)
		{
			Serial.read();
		}

		/* clear the buffer */
		for (int i = 0; i < sizeof(serialBuffer); i++)
		{
			serialBuffer[i] = 0x00;
		}

		/* reset the buffer pointer */
		serialBufferPtr = &serialBuffer[0];
	}
}
