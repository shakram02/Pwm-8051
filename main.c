#include <at89c51xd2.h>
#include "freq_helper.h"

sbit led = P2^0; /* Use P2.0 for your output generated signal. */
sbit error_led = P2^6;
sbit output_ctl = P2^7; /* P2.7 to turn the output signal ON and OFF */

/* Adjust mode */
sbit adjust_mode= P2^5;
sbit adjust_led = P2^4;

void adjust_state();
void output_half_cycle(TIMER_VALS* tv);
void error_state();
void calc_micros_config(int micros,TIMER_VALS* tv);
char duty_cycle(int, unsigned char);

unsigned char from_bcd(unsigned char);
	
/* ON/OFF values for duty cycle */
TIMER_VALS tv_on;
TIMER_VALS tv_off;

void init(){
	P1 = 0;
	P2 = 0;
	/*
		P3 will be used to take the input frequency, ranges 10~990 MHz -> (1 ~ 99) in BCD * 10
	*/
	P3 = 0;
	output_ctl = 1;
}

void main() using 0{
	/* Timer 0 mode 1 */
	int mics = to_micros(500);
	int freq = 800;
	unsigned char percent = 50;
	init();
	TMOD 	= 0x01;
	
	if(duty_cycle(freq ,percent)){
		error_state();
	}
	
	/* Port1 is used as input */
	TR0 = 1;
	led = 1;
	while(1){
		/*output_ctl*/
		while (!adjust_mode){
			/* Operate the timer using tv_on when it overflows, switch to tv_off */
			output_half_cycle(&tv_on);
			output_half_cycle(&tv_off);
		}
		
		// The loop is broken only when adjust_state is high
		adjust_state();		
		adjust_led = 0;
	}
}



/*
	Generates the timer values for a given
	frequency and duty cycle
*/
char duty_cycle(int freq, unsigned char percent){
	int low_micros;
	int micros = to_micros(freq);
	
	/* Simple way to avoid floating point arithmetic */
	/* 0.2 * 1000 -> 20 * (1000/100) */
	int high_micros  = percent * (micros/100);
	
	/* Use the chars to indicate error */
	/* Calculate the ON / OFF time in us */
	/* CALL_OVERHEAD is used to adjust the frequency */
	
	OV = 0;	// Check for overflow
	
	calc_micros_config(high_micros - CALL_OVERHEAD,&tv_on);
	low_micros = micros - high_micros;	/* Use cached values, a bit poorer code readability */
	calc_micros_config(low_micros  - CALL_OVERHEAD,&tv_off);
	
	if (OV)
	{
		/* Overflow, Indicate error */
		OV = 0;
		return -1;
	}
	
	return 0;
}

/*
	Generates half the cycle of the square wave
*/
void output_half_cycle(TIMER_VALS* tv){
	TF0 = 0;
	TH0 = tv->th;
	TL0 = tv->tl;
	while(TF0 == 0);
	led = ~led;			
}

/*
	Sets up the PWM generator
*/
void adjust_state(){
	/* Read P1 -> convert from BCD */
	unsigned char percent = from_bcd((unsigned char)P1);
	/* Read the frequency */
	int freq = from_bcd((unsigned char)P3)*10;	// Scale by 10 to provide a better range
	
	if (percent > (unsigned char)100)error_state();
	adjust_led = 1;
	
	if(duty_cycle(freq,percent)){
		error_state();
	}
}


/*
	Unpack BCD value
*/
unsigned char from_bcd(unsigned char val){
	
	/* value = (higher nibble*10) + (lower nibble) */
	
	unsigned char unpacked = ( (val & (unsigned char)0xF0) >> NIBBLE_SIZE)*10 + (val & (unsigned char)0x0F);
	return unpacked;
}
/* Error state, do nothing until reset */
void error_state(){
		TMOD = 0x00;
		error_led = 1;
		while(1);
}