#include "pid.h"
#include "main.h"
#include <avr/io.h>


extern uint32_t uptime_x_0_1s;
extern unsigned int rps;
extern uint8_t t1;
extern state_str_s GLOBAL_STATE;
;
// K_T = 1/2
#define K_T_A 1
#define K_T_B 2

// K_RPS = 1/5
#define K_RPS_A 1
#define K_RPS_B 5 
void do_pid()
{
	static uint32_t pid_ts;
	//static uint8_t valid_pwm = 255;
	if (uptime_x_0_1s-pid_ts < 10) return;
	pid_ts = uptime_x_0_1s;
	if (GLOBAL_STATE.ID_STATE == LED_ON_0 || GLOBAL_STATE.ID_STATE == LED_ON_1 || GLOBAL_STATE.ID_STATE == LED_OFF_0 )
	{} else { return;};
	//if (rps > 20) valid_pwm = OCR2;
	int pwm_val = OCR2;

	int dt = t1 - GOAL_T;
	int d_rps = GOAL_RPS - rps;
	int d_pwm_t = (dt * K_T_A ) / K_T_B;
	if (d_pwm_t < -5) {
		d_pwm_t = -5;
	};
	int d_pwm_rps = (d_rps * K_RPS_A) / K_RPS_B;
	
	int pwm_val_t = pwm_val + d_pwm_t;
	int pwm_val_rps = pwm_val + d_pwm_rps;
	int d_pwm;
	if (pwm_val_t > pwm_val_rps)
		{
			d_pwm = d_pwm_t;
		} else {
			d_pwm = d_pwm_rps;
		};
	pwm_val += d_pwm;
	if (pwm_val > 255) pwm_val = 255;
	if (pwm_val <0 ) pwm_val = 0;
	writeSerial("do_pid(): d_t=");writeInt(dt); writeSerial("  d_pwm_t="); writeInt(d_pwm_t);writeSerial("\n");
	writeSerial("do_pid(): d_rps=");writeInt(d_rps); writeSerial("  d_pwm_rps="); writeInt(d_pwm_rps);writeSerial("\n");

	writeSerial("do_pid(): use d_pwm=");writeInt(d_pwm); writeSerial("  pwm_val="); writeInt(pwm_val);writeSerial("\n");
	
	OCR2 = pwm_val;
}
