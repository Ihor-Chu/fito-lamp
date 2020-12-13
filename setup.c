/*
 * setup.c
 * 
 * Copyright 2020 igor <igor@igor-chumak>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#include "setup.h"
void setup()
{
	DDRB |= LED_INFO_PIN; // OUTPUT
	DDRD |= LED_PIN;
	PORTB |= BTN_PIN;
	
// PWM	
	DDRB |= PWM_FAN_PIN;
	
	OCR2 = 255;
    // set PWM for 100% duty cycle
    TCCR2 |= (1 << COM21);
    TCCR2 |= (1 << COM20);
    // set inverting mode

    TCCR2 |= (1 << WGM21) | (1 << WGM20);
    // set fast PWM Mode

    TCCR2 |= (1 << CS20);
    // set prescaler to 0 and starts PWM
    
// PWM

	
// FAN interrupt
	SREG |= (1<<7);
	PORTD |= TAHO_FAN_PIN; // pull-up
	GIMSK |= _BV(INT1);
	MCUCR |= _BV(ISC11); // Прерывание вызывается по возрастающему фронту сигнала на входе INT
// UART
	UBRRH = (unsigned char) ((MY_UBRR) >> 8);
	UBRRL = (unsigned char) (MY_UBRR);
	UCSRB = (1<<TXEN);
	UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);  

// 16000000/256/6250 = 10 hz
// timer 1, CTC, 256 prescaler
	TCCR1B |= (1 << CS12) | (1 << WGM12);
	TIMSK |= (1 << OCF1A); // interrupt 
	OCR1A = 6250;

// adc https://narodstream.ru/avr-urok-22-izuchaem-acp-chast-1/
	ADCSRA |= (1<<ADEN);  // Разрешение использования АЦП
	ADCSRA |=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);//Делитель 128 = 125 кГц 
	ADMUX  |= (1<<REFS1)|(1<<REFS0)|(1<<ADLAR); //Внутренний Источник ОН 2,56в, adc0, use high 8b (ADCH)
}

