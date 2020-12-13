#include <avr/io.h>

#define LED_INFO_PIN _BV(PB1)
// #define LED_PIN 7

#define LED_PIN _BV(PD7)
#define PWM_FAN_PIN _BV(PB3)

#define TAHO_FAN_PIN _BV(PD3)
#define BTN_PIN	_BV(PB0)

#define BAUD 38400
#define MY_UBRR (F_CPU/16/BAUD - 1)
void setup();
