#pragma once
#include <intrinsics.h>
#include <msp430fr6989.h>

/*
 *  Author: Milton Pagan Soto
 *  LCD control library
 *  Operating in 4-bit mode
 */

#define F_CLK 1000000
#define MS_CYCLES 1000
#define US_CYCLES 1

#define RS BIT4
#define EN BIT6

#define D0 BIT0
#define D1 BIT1
#define D2 BIT2
#define D3 BIT3
#define D4 BIT4
#define D5 BIT5
#define D6 BIT6
#define D7 BIT7

#define LCD_DATA_DIR P9DIR
#define LCD_DATA_OUT P9OUT
#define LCD_CONTROL_OUT P9OUT
#define LCD_CONTROL_DIR P9DIR

//#define RS_H P2OUT |= BIT4 //  RS high
//#define RS_L P1OUT &= ~BIT4 // RS low
//#define R P2OUT |= BIT5 //  Read
//#define W P2OUT &= ~BIT5 // Write
//#define E_H P2OUT |= BIT6 // Enable high
//#define E_L P2OUT &= ~BIT6 // Enable low

void display_init();
void enable_cycle();
void send_command(unsigned char);
void send_character(unsigned char character);
void send_nibble(unsigned char);
void send_byte(unsigned char);
void send_string(char[], unsigned char);
void set_cursor(unsigned char, unsigned char);
void clear_display();
void display_off();
