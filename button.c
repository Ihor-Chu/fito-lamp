#include "button.h"
#include <avr/interrupt.h>
#include "main.h"

extern uint32_t uptime_x_0_1s;

state_btn button_states[2];

uint8_t tcnt1_cmp(uint16_t from, uint16_t to)
{
	if (from < to)
	{
		if (to - from  >= 1250)
		{ return 1;}
		else { return 0;};
	} else { // to < from; 
		 if ( 6250 - from + to >= 1250) 
		 {return 1; } else {return 0;};
		};
};

void query_button()
{
static uint32_t click_time;
static uint16_t click_time_timer;
static state_btn button_state = BTN_UNKNOWN;
  if (PINB & BTN_PIN)
   { // released
	   	if (button_state == BTN_RELEASED) 
		{
			cli();
			if (tcnt1_cmp(click_time_timer, TCNT1) || (click_time - uptime_x_0_1s >1))
			{
				button_states[1] = button_states[0];
				button_states[0] = BTN_RELEASED;
			};
			sei();
		} else {
			cli();
			click_time = uptime_x_0_1s;
			click_time_timer = TCNT1; // 1 tick == 16 uS; 20 ms = 1250 tick
			sei();
			button_state = BTN_RELEASED;
		};
	} else { //pressed
		if (button_state == BTN_PRESSED) 
		{
			cli();
			if (tcnt1_cmp(click_time_timer, TCNT1) || (click_time - uptime_x_0_1s >1))
			{
				button_states[1] = button_states[0];
				button_states[0] = BTN_PRESSED;
			};
			sei();
		} else {
			cli();
			click_time = uptime_x_0_1s;
			click_time_timer = TCNT1; // 1 tick == 16 uS; 20 ms = 1250 tick
			sei();
			button_state = BTN_PRESSED;
		};
	};

}
