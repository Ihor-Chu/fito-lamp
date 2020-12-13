#include <avr/io.h>

typedef enum f_state {
	UNKNOWN = 0,
	LED_ON_0, // start fan
	LED_ON_1, // led_on
	LED_OFF_0, // led off, fan working
	LED_OFF_1, // stop fan and led
	ERROR
	
} state_f;

#define STATE_DARK (1<<7)
#define STATE_LIGHT (1<<6)
#define STATE_DARK_COMPLETED (1<<5)
#define STATE_LIGHT_COMPLETED (1<<4)
#define STATE_WAIT_COMPLETED (1<<3)
#define STATE_WAIT_NEED (1<<2)
#define STATE_EXPO_ON 1
typedef struct state_str
{
	state_f ID_STATE;
	uint8_t state_flags;
	uint32_t ID_STATE_TS; 
} state_str_s;

void writeSerial(char* str);
void writeInt(int x);
