#include <at89c51xd2.h>
#include <stdio.h>

#define PERIOD_MICROS (22.1184/12)	/* 8051 cycle time in microseconds */
#define BYTE_SIZE 8
#define TO_MIRCOS 1000000	/* 10^6 to convert time in secs to micros */
#define FREQ_500HZ_MICROS 2000	/* 500Hz frequency in microseconds */
sbit led = P2^0; /* Use P2.0 for your output generated signal. */
sbit error_led = P2^6;
sbit output_ctl = P2^7; /* P2.7 to turn the output signal ON and OFF */

typedef struct TIMER_VALS{
	short th;
	short tl;
}TIMER_VALS;

int calc_micros_config(int micros,TIMER_VALS* tv);
char duty_cycle(unsigned char percent);
int to_micros(int);
void error_state();

/* ON/OFF values for duty cycle */
TIMER_VALS tv_on;
TIMER_VALS tv_off;

void main() using 0{
	TIMER_VALS tv;
	/* Timer 0 mode 1 */
	TMOD 	= 0x01;
	
		/* For testing correctness */
	if (calc_micros_config(2000,&tv))
	{
		error_state();
	}
	
	/* Mode 1 of duty cycle -> 25% */
//	if(duty_cycle(1)){
//		error_state();
//	}
	
	/* Port1 is used as input */
	TR0 = 1;
	
	while (1){
		
		if(!output_ctl)continue;
		/* Operate the timer using tv_on when it overflows, switch to tv_off */
		TF0 = 0;
		TH0 = tv.th;
		TL0 = tv.tl;
		while(TF0 == 0);
		led = ~led;		
		
		TF0 = 0;
		TH0 = tv.th;
		TL0 = tv.tl;
		while(TF0 == 0);
		led = ~led;			
		
	}
}

int to_micros(int hertz){
	/* Not working, values are zeroed */
	double val = (1/hertz);
	unsigned long int	micros= val * TO_MIRCOS;
	return micros;
}

/*
	Calculates the timer values for the supplied mode of duty
	cycle, the code is ugly because I couldn't create the calculations
	with double values as the values in double/long operations 
	were zeroed for a reason that I don't know.
*/
char duty_cycle(unsigned char mode){
	char on=1,off=1;
	
	if(mode == 1)
	{
		/*20%*/
		on = calc_micros_config(400,&tv_on);
		off = calc_micros_config(1600,&tv_off);
	}
	else if(mode == 2)
	{
		/*25%*/
		on = calc_micros_config(500,&tv_on);
		off = calc_micros_config(1500,&tv_off);
	}
	else if(mode == 3)
	{
		/*50%*/
		on = calc_micros_config(800,&tv_on);
		off = calc_micros_config(800,&tv_off);
	}
	else if(mode == 4)
	{
		/*75%*/
		on = calc_micros_config(1500,&tv_on);
		off = calc_micros_config(500,&tv_off);
	}
	else if(mode == 5)
		{
		/*80%*/
		on = calc_micros_config(1600,&tv_on);
		off = calc_micros_config(400,&tv_off);
	}		
	
	return on || off;
}

/* Calculates TH and TL values to generate a given delay in microseconds */
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

void error_state(){
		TMOD = 0x00;
		error_led = 1;
		while(1);
}