#include <at89c51xd2.h>

#define PERIOD_MICROS (22.1184/12)	/* 8051 cycle time in microseconds */
#define BYTE_SIZE 8
#define TO_MIRCOS 1000000	/* 10^6 to convert time in secs to micros */
#define FREQ_500HZ_MICROS 2000	/* 500Hz frequency in microseconds */
/* 
Adjust the frequencies using this constant, 
roughly approximated from listing file with assembly mapping
*/
#define CALL_OVERHEAD 30 
sbit led = P2^0; /* Use P2.0 for your output generated signal. */
sbit error_led = P2^6;
sbit output_ctl = P2^7; /* P2.7 to turn the output signal ON and OFF */

/* Adjust mode */
sbit adjust_mode= P2^5;
sbit adjust_led = P2^4;

typedef struct TIMER_VALS{
	unsigned char th;
	unsigned char tl;
}TIMER_VALS;

void adjust_state();
void output_half_cycle(TIMER_VALS* tv);
void error_state();
int calc_micros_config(int micros,TIMER_VALS* tv);
int to_micros(int);
char duty_cycle(int, unsigned char);
unsigned char from_bcd(unsigned char);
	
/* ON/OFF values for duty cycle */
TIMER_VALS tv_on;
TIMER_VALS tv_off;

void init(){
	P1 = 0;
	P2 = 0;
	output_ctl = 1;
}

void main() using 0{
	/* Timer 0 mode 1 */
	int mics = to_micros(500);
	init();
	TMOD 	= 0x01;
	
	// 500Hz 50% duty cycle
	if(duty_cycle(500,50)){
		error_state();
	}
	
	/* Port1 is used as input */
	TR0 = 1;
	led = 0;
	while(1){
		/*output_ctl*/
		while (!adjust_mode){
			/* Operate the timer using tv_on when it overflows, switch to tv_off */
			output_half_cycle(&tv_off);
			output_half_cycle(&tv_on);
		}
		
		// The loop is broken only when adjust_state is high
		adjust_state();		
	}
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
	Converts a frequency in hertz to us
*/
int to_micros(int hertz){
	
	/* Not working, values are zeroed */
	double val = (1.0/(double)hertz);
	unsigned long int	micros = (unsigned long int)(val * TO_MIRCOS);
	
	return (int)micros;
}


/*
	Sets up the PWM generator
*/
void adjust_state(){
	
	/* Read P1 -> convert from BCD */
	unsigned char percent = from_bcd(P1);
	
	/* Setup the duty cycle */ 
	if(duty_cycle(500,percent)){
		error_state();
	}
	
	/* TODO: Read the frequency */
}


/*
	Unpack BCD value
*/
unsigned char from_bcd(unsigned char val){
	
	/* value = (higher nibble*10) + (lower nibble) */
	unsigned char unpacked = ((val & (unsigned char)0xFF)*10) + (val & (unsigned char)0xFF);
	
	if (unpacked > (unsigned char)100)error_state();
	return unpacked;
}


/*
	Generates the timer values for a given
	frequency and duty cycle
*/
char duty_cycle(int freq, unsigned char percent){
	
	int micros = to_micros(freq);
	
	/* Simple way to avoid floating point arithmetic */
	/* 0.2 * 1000 -> 20 * (1000/100) */
	int high_micros  = percent * (micros/100);
	
	/* Use the chars to indicate error */
	/* Calculate the ON / OFF time in us */
	/* CALL_OVERHEAD is used to adjust the frequency */
	char on  = calc_micros_config(high_micros - CALL_OVERHEAD,&tv_on);
	int low_micros = micros - high_micros;	/* Use cached values, a bit poorer code readability */
	
	char off = calc_micros_config(low_micros  - CALL_OVERHEAD,&tv_off);
	return on | off;
}


/* Calculates TH and TL values to generate a given delay in us */
int calc_micros_config(int micros,TIMER_VALS* tv){
	
	/* Get the number of increments required */
	int incr = micros * PERIOD_MICROS;
	int start_val;
	
	if (OV)
	{
		/* Overflow, Indicate error */
		OV = 0;
		return -1;
	}
	
	start_val = 0xFFFF - incr;
	tv->tl = start_val & 0x00FF;	/* The LOW part of the delay */
	tv->th = ((start_val & 0xFF00) >> BYTE_SIZE);	/* The HIGH part of the delay */
		
	return 0;
}


/* Error state, do nothing until reset */
void error_state(){
		TMOD = 0x00;
		error_led = 1;
		while(1);
}