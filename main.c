
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h> 

#include "setup.h"
#include "adc.h"
#include "main.h"
#include "button.h"
#include "pid.h"


#define REAL_BUILD

#ifdef REAL_BUILD
#define WAIT_MIN_TIME (288000) //8H
#define DARKNESS_TIME (72000) // 2 H
#define BRIGHTNESS_TIME (18000) // 30 min
//#define EXPO_TIME (36000) // 1 H
#define EXPO_TIME (288000) //8H
#else
#define WAIT_MIN_TIME (288000) //8H
#define DARKNESS_TIME (10 * 5) //5s
#define BRIGHTNESS_TIME (10 * 5) //5s
#define EXPO_TIME (10*10) //10s
#endif

#define START_FAN_TIMEOUT 30 // 3s
#define STOP_FAN_TIMEOUT 40 // 4s
#define OVERHEAT_FAN_TIMEOUT 1200 // 2 m

#define BRIGHT_TRESHOLD 240
#define DARK_TRESHOLD 250
			
char print_buf[10];
uint32_t uptime_x_0_1s = 0;
unsigned int fan_cnt = 0;
unsigned int rps = 0;
uint8_t history_counter=0;


state_str_s GLOBAL_STATE;
extern state_btn button_states[];

uint16_t ts_fan_interrupt;
ISR (INT1_vect)
{
    /* interrupt code here */
    
    if (TCNT1 - ts_fan_interrupt <10) return;
    ts_fan_interrupt = TCNT1;
    fan_cnt ++;

}
ISR (TIMER1_COMPA_vect)
{
    // action to be done every 0.1 sec
    uptime_x_0_1s++;
    ts_fan_interrupt = 0;
}

void set_state(state_f new_state)
{
	GLOBAL_STATE.ID_STATE = new_state;
	GLOBAL_STATE.ID_STATE_TS = uptime_x_0_1s;
};

void writeSerial(char* str)
{
	for(int i=0;i<strlen(str); i++)	
	{
		while(!(UCSRA&(1<<UDRE))){}; // wait ready of port
		UDR = str[i];
	};
}

void writeInt(int x)
{
	char * res = itoa(x, print_buf, 10);
	writeSerial(res);
}
void writeUint32_t(uint32_t x)
{
	char * res = ultoa(x, print_buf, 10);
	writeSerial(res);
}
void writeHex(int x)
{
	writeSerial("0x");
	char * res = itoa(x, print_buf, 16);
	writeSerial(res);
}

void do_fan_rps()
{
	static uint32_t rps_ts;
	static unsigned int rps_hist[4];
	
	unsigned int fan_cnt_;
	int dt = uptime_x_0_1s - rps_ts;
	if ( dt < 10) return;
//	writeSerial("fan_cnt="); writeInt(fan_cnt); writeSerial(" dt="); writeInt(dt);
	cli();
	fan_cnt_ = fan_cnt;
	fan_cnt = 0;
	sei();
	rps = fan_cnt_ * 10 /dt ;
	
	rps_ts = uptime_x_0_1s;
	
//	writeSerial(" fan_cnt="); writeInt(fan_cnt);writeSerial(" rps=");writeInt(rps);;writeSerial("\n");
	rps_hist[3]=rps_hist[2];
	rps_hist[2]=rps_hist[1];
	rps_hist[1]=rps_hist[0];
	rps_hist[0]=rps;
	if (history_counter < sizeof(rps_hist)/sizeof(rps_hist[0])) history_counter++;
	unsigned int avg_rps = 0;
	for (uint8_t i = 0; i < history_counter; i++)
	{
		//writeInt(rps_hist[i]);  writeSerial(";");
		avg_rps += rps_hist[i];
		
	};
	if (history_counter ==0) history_counter=1;
	avg_rps = avg_rps / history_counter;
//	writeSerial("AVG rps=");writeInt(avg_rps);writeSerial(" cnt=");writeInt(history_counter);writeSerial("\n");
	if (OCR2 > 0 && avg_rps < 20 && history_counter == sizeof(rps_hist)/sizeof(rps_hist[0]))
	{
		writeSerial("RPS too slow\n");
		if (GLOBAL_STATE.ID_STATE == LED_ON_0 || GLOBAL_STATE.ID_STATE == LED_ON_1 || GLOBAL_STATE.ID_STATE == LED_OFF_0)
		{
			set_state(ERROR);
	    };
	}; 
};


uint8_t a0,t1;

void do_adc()
{
	static uint32_t adc_ts;
	if (uptime_x_0_1s-adc_ts < 9) return;
	a0=ADC_read(0);
	// фото; 255 - темно; 150 - вечер, 50 - яркий свет
	t1= adc_to_t(ADC_read(1));

	adc_ts = uptime_x_0_1s;
}



void do_show_state()
{

	static uint32_t _ts;
	static uint16_t bitmask = 1;
	
	if (uptime_x_0_1s == _ts) return;
	_ts = uptime_x_0_1s;
	uint16_t blink_form = 0b1; // 1 короткая вспышка - дежурный режим
	if (GLOBAL_STATE.state_flags == STATE_LIGHT) blink_form = 0b11; // 1 длинная вспышка - просто свет
	if (GLOBAL_STATE.state_flags & STATE_DARK && ! (GLOBAL_STATE.state_flags & STATE_DARK_COMPLETED))
		{
			blink_form = 0b101; // 2 короткие - таймаут в темноте
		};
	if (GLOBAL_STATE.state_flags & STATE_DARK && (GLOBAL_STATE.state_flags & STATE_DARK_COMPLETED))
		{
			blink_form = 0b11011;  // 2 удлинненные - таймаут в темноте выдержан
		};
	if ((GLOBAL_STATE.state_flags & STATE_DARK_COMPLETED) && (GLOBAL_STATE.state_flags & STATE_LIGHT))
		{
			blink_form = 0b10101;  // 3 вспышки - таймайт на свету
		};
	if (GLOBAL_STATE.ID_STATE == ERROR)
		{
			blink_form = 0xaaaa; // частые вспышки - ошибка
		};
	if ((GLOBAL_STATE.state_flags & STATE_DARK_COMPLETED) && (GLOBAL_STATE.state_flags & STATE_LIGHT_COMPLETED) && (GLOBAL_STATE.state_flags & STATE_WAIT_NEED))
		{  // таймаут после автовыключения - 3 удлинненные
			blink_form = 0b11011011;
		};
	if (GLOBAL_STATE.ID_STATE == LED_ON_0 || GLOBAL_STATE.ID_STATE == LED_ON_1)
		{
			blink_form = ~0b1; // вкл свет - просто моргает
		};

	if (blink_form & bitmask)
	{
		PORTB |= LED_INFO_PIN;
	} else {
		PORTB &= ~LED_INFO_PIN;
	};
	
	bitmask = (bitmask << 1);
	if (bitmask == 0) bitmask = 1;
}


uint8_t light_around = 0;
uint32_t light_ts;

void do_light_measure()
{	static uint32_t light_measure_ts;
	if (GLOBAL_STATE.ID_STATE != LED_OFF_1) return;
	if (uptime_x_0_1s - light_measure_ts < 5*10) return;
	light_measure_ts = uptime_x_0_1s;
	if (a0 > DARK_TRESHOLD)
		{ // in darkness
			if ((GLOBAL_STATE.state_flags & STATE_DARK) == 0) light_ts = uptime_x_0_1s;
			GLOBAL_STATE.state_flags &= ~STATE_LIGHT;
			GLOBAL_STATE.state_flags |= STATE_DARK;
			writeSerial("In darkness: "); writeUint32_t(uptime_x_0_1s - light_ts);writeSerial(" of ");writeUint32_t(DARKNESS_TIME);
			if (uptime_x_0_1s - light_ts > DARKNESS_TIME)
				{
					GLOBAL_STATE.state_flags |= STATE_DARK_COMPLETED;
					writeSerial(" done");
				};
			writeSerial("\n");
		}
	else if (a0 < BRIGHT_TRESHOLD)
		{ // in bright
			if ((GLOBAL_STATE.state_flags & STATE_LIGHT) == 0) light_ts = uptime_x_0_1s;
			GLOBAL_STATE.state_flags &= ~STATE_DARK;
			GLOBAL_STATE.state_flags |= STATE_LIGHT;
			if (GLOBAL_STATE.state_flags & STATE_DARK_COMPLETED)
			{
			writeSerial("In bright: "); writeUint32_t(uptime_x_0_1s - light_ts);writeSerial(" of ");writeUint32_t(BRIGHTNESS_TIME);
			if (uptime_x_0_1s - light_ts > BRIGHTNESS_TIME )
				{
					GLOBAL_STATE.state_flags |= STATE_LIGHT_COMPLETED;
					writeSerial(" done");
				};
			writeSerial("\n");
			} else {
				writeSerial("In bright: "); writeUint32_t(uptime_x_0_1s - light_ts);writeSerial(" but darkness ");writeUint32_t(DARKNESS_TIME);writeSerial(" uncompleted\n");
			};
		}
	else { // reset uncompleted
		if ((GLOBAL_STATE.state_flags & STATE_DARK_COMPLETED) == 0)
			{GLOBAL_STATE.state_flags &= ~STATE_DARK;};
		if ((GLOBAL_STATE.state_flags & STATE_LIGHT_COMPLETED) == 0)
			{GLOBAL_STATE.state_flags &= ~STATE_LIGHT;};

	};

}

void do_auto_on()
{
	static uint32_t auto_on_ts;
	if (GLOBAL_STATE.ID_STATE != LED_OFF_1) return;
	if (uptime_x_0_1s - auto_on_ts < 10*10) return;
	auto_on_ts = uptime_x_0_1s;
	if (GLOBAL_STATE.state_flags & STATE_WAIT_NEED && (uptime_x_0_1s - GLOBAL_STATE.ID_STATE_TS > WAIT_MIN_TIME))
		{
			GLOBAL_STATE.state_flags |= STATE_WAIT_COMPLETED;
		};
	if ( ! (GLOBAL_STATE.state_flags & STATE_DARK_COMPLETED)) return;
	if ( ! (GLOBAL_STATE.state_flags & STATE_LIGHT_COMPLETED)) return;
	if ((! (GLOBAL_STATE.state_flags & STATE_WAIT_NEED)) || 
	       ((GLOBAL_STATE.state_flags & STATE_WAIT_NEED) && (GLOBAL_STATE.state_flags & STATE_WAIT_COMPLETED))
	       )
		{
			GLOBAL_STATE.state_flags = STATE_EXPO_ON;
		};
}

void do_print_info()
{
	static uint32_t print_ts;
	if (uptime_x_0_1s-print_ts < 20) return;
	print_ts = uptime_x_0_1s;
	writeSerial("a0=");
	writeInt(a0);
	writeSerial("; t1=");
	writeInt(t1);
	writeSerial(" On ");
	writeUint32_t(uptime_x_0_1s/10);
	writeSerial(" fan rpm ");
	writeInt(rps);
	writeSerial(" state_flags ");
	writeHex(GLOBAL_STATE.state_flags);
	writeSerial(" STATE ");
	writeInt(GLOBAL_STATE.ID_STATE);
	writeSerial(" OCR2 ");
	writeInt(OCR2);
	writeSerial("\n");

};



int main(void)
{

	setup();
	sei();
	GLOBAL_STATE.state_flags = 0;
	set_state(LED_OFF_1);
	wdt_enable(WDTO_2S);

    while(1)
    {
		wdt_reset();
		do_fan_rps();
		do_adc();
		query_button();
		if (button_states[1] == BTN_PRESSED && button_states[0] == BTN_RELEASED)
		{
			button_states[1] = BTN_UNKNOWN;
			GLOBAL_STATE.state_flags ^= STATE_EXPO_ON;
			history_counter=0;
			GLOBAL_STATE.state_flags &= STATE_EXPO_ON; // reset bright detect
		};
		do_light_measure();
		do_auto_on();
		
        do_print_info();
        
        if (GLOBAL_STATE.state_flags & STATE_EXPO_ON)
        {
			if (GLOBAL_STATE.ID_STATE == ERROR)
			{}
			else if (GLOBAL_STATE.ID_STATE == LED_ON_1)
			{
				PORTD |= LED_PIN;
				if (uptime_x_0_1s - GLOBAL_STATE.ID_STATE_TS > EXPO_TIME)
				{ //auto off
					GLOBAL_STATE.state_flags &= ~STATE_EXPO_ON;
					GLOBAL_STATE.state_flags &= ~STATE_DARK_COMPLETED;
					GLOBAL_STATE.state_flags &= ~STATE_LIGHT_COMPLETED;
					GLOBAL_STATE.state_flags |= STATE_WAIT_NEED;
					writeSerial("AUTO OFF\n");
				};
			}
			else if (GLOBAL_STATE.ID_STATE == LED_ON_0)
			{
				if (uptime_x_0_1s - GLOBAL_STATE.ID_STATE_TS > START_FAN_TIMEOUT) 
				{
					set_state(LED_ON_1);
					history_counter=0;
					if (rps < 30) {
						set_state(ERROR);
					};
				};
			} else 
			{
				set_state(LED_ON_0);
				OCR2 = 255;
				history_counter=0;
			};
			  
		}
		else {
			if (GLOBAL_STATE.ID_STATE == LED_OFF_1) 
			{
				OCR2 = 0;
			}
			else if (GLOBAL_STATE.ID_STATE == LED_OFF_0)
			{
				if (uptime_x_0_1s - GLOBAL_STATE.ID_STATE_TS > STOP_FAN_TIMEOUT) 
				{
					set_state(LED_OFF_1);
					history_counter=0;
				};
			}
			else {
				set_state(LED_OFF_0);
				PORTD &= ~LED_PIN;
				history_counter=0;
			};		
		};
		do_pid();
		if (t1 > 60) {
				set_state(ERROR);
				OCR2 = 255; // усиленное охлаждение 1 мин
		};
		if (GLOBAL_STATE.ID_STATE == ERROR)
		{
			PORTD &= ~LED_PIN;
			if (uptime_x_0_1s - GLOBAL_STATE.ID_STATE_TS > OVERHEAT_FAN_TIMEOUT)
			   {
				   OCR2 = 0;
			   };
		};
		do_show_state();
    } // while loop
}
