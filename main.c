#include <msp430.h> 


#define SDA BIT0
#define SCL BIT1

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// Scheduling system setup

	// I2C Setup
	P4SEL1 |= SDA | SCL;
	P4SEL0 &= ~SDA & ~SCL;

	// Dispensing mechanism setup


	// Analytics and communication setup


	return 0;
}
