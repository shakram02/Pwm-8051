#ifndef FREQ_HELPER
#define FREQ_HELPER

#define PERIOD_MICROS (22.1184/12)	/* 8051 cycle time in microseconds */
#define BYTE_SIZE 8
#define NIBBLE_SIZE 4
#define TO_MIRCOS 1000000	/* 10^6 to convert time in secs to micros */
#define FREQ_500HZ_MICROS 2000	/* 500Hz frequency in microseconds */

/* 
Adjust the frequencies using this constant, 
roughly approximated from listing file with assembly mapping
*/
#define CALL_OVERHEAD 30 

typedef struct TIMER_VALS{
	unsigned char th;
	unsigned char tl;
}TIMER_VALS;


/*
	Converts a frequency in hertz to us
*/
int to_micros(int hertz){
	
	/* Not working, values are zeroed */
	double val = (1.0/(double)hertz);
	unsigned long int	micros = (unsigned long int)(val * TO_MIRCOS);
	
	return (int)micros;
}

/* Calculates TH and TL values to generate a given delay in us */
void calc_micros_config(int micros,TIMER_VALS* tv){
	
	/* Get the number of increments required */
	int incr = micros * PERIOD_MICROS;
	int start_val;
	
	start_val = 0xFFFF - incr;
	tv->tl = start_val & 0x00FF;	/* The LOW part of the delay */
	tv->th = ((start_val & 0xFF00) >> BYTE_SIZE);	/* The HIGH part of the delay */	
}

#endif