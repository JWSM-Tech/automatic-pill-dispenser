#include <msp430fr6989.h>
#include "init.h"
/*
 * init.c
 *
 *  Created on: Apr 6, 2021
 *  Author: Milton Pagan
 */

void init_clock(void)
{
    // Initializes LFXTCLK used for the RTC
    // For MCLK and SMCLK default DCO configuration is used (1MHz)

    PJSEL0 = BIT4 | BIT5; // Initialize LFXT pins

    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8; // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;    // Enable LFXT
    do
    {
        CSCTL5 &= ~LFXTOFFG; // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG); // Test oscillator fault flag
    CSCTL0_H = 0;              // Lock CS registers
}

void init_sched_syst(void)
{
    // Scheduling system setup

    //*** BUZZER SETUP ***

    // Pin
    P1DIR |= BUZZER;
    P1OUT &= ~BUZZER;
    P1SEL1 |= BUZZER;
    P1SEL0 |= BUZZER;

    // Timer

    // TODO: Initialize compare register CCRs
    // TODO: Might need another timer or reg to time how long the buzzer is one
    // Could also use RTC
    // Use CCR1

    //TA0CCTL1 = OUTMOD_4; // toggle
    TA0CTL = (TACLR | TASSEL__SMCLK | ID__1 | MC__CONTINUOUS);
    TA0CCR1 = 0;

    //*** RTC SETUP ***
    // *** BUTTONS SETUP ***

    P2DIR &= ~(UP | DOWN | ENTER | BACK);
    P2IFG &= ~(UP | DOWN | ENTER | BACK);
    P2IES |= UP;
    P2IES &= ~(DOWN | ENTER | BACK);
    P2IE |= UP | DOWN | ENTER | BACK;

    // Configure RTC_C
    // parametros de a~o, mes, dia, dia emana, hora, minutos
    RTCCTL0_H = RTCKEY_H;                     // Unlock RTC
    RTCCTL0_L = RTCTEVIE | RTCRDYIE | RTCAIE; // enable RTC read ready interrupt
                                              // enable RTC time event interrupt

    RTCCTL1 = RTCBCD | RTCHOLD | RTCMODE; // RTC enable, BCD mode, RTC hold

    RTCYEAR = 0x2021; // Year = 0x2021
    RTCMON = 0x4;     // Month = 0x04 = April
    RTCDAY = 0x21;    // Day = 0x13 = 13
    RTCDOW = 0x03;    // Day of week = 0x02 = tuesday
    RTCHOUR = 0x08;   // Hour = 0x10
    RTCMIN = 0x56;    // Minute = 0x00
    RTCSEC = 0x00;    // Seconds = 0x00

    RTCCTL1 &= ~(RTCHOLD); // Start RTC

    // *** I2C SETUP ***
    // NOTE: eUSCI1_B is used

    // Pins
    P4SEL1 |= SDA | SCL;
    P4SEL0 &= ~SDA & ~SCL;

    // I2C registers
}

void init_disp_mech(void)
{
    // Dispensing mechanism setup

    // Pin for IR
    P2DIR &= ~IR; //set as input
    P2REN |= IR;  //enable resistor
    P2OUT |= IR;  //set output high to select pull up configuration
    P2IES |= IR;  //select edge from high to low
    P2IFG &= ~IR; //clear any pending interrupt requests
    P2IE |= IR;   //enable interrupt

    //configure PWM output pins for servo motors
    P2DIR |= DISPENSER_SERVO + LINEAR_SERVO;   //set as output
    P2OUT &= ~DISPENSER_SERVO & ~LINEAR_SERVO; //initialize as logic low
    //select timer compare functionality
    P2SEL1 &= ~DISPENSER_SERVO & ~LINEAR_SERVO;
    P2SEL0 |= DISPENSER_SERVO + LINEAR_SERVO;

    // stepper pins set up
    P3DIR |= BIT0 + BIT1 + BIT2 + BIT3;     //set as output
    P3OUT &= ~BIT0 & ~BIT1 & ~BIT2 & ~BIT3; //initialize as logic low

    // Timers

    // Timer for PWM
    TB0CCTL5 = OUTMOD_7; //select reset/set (dispenser servo)
    TB0CCTL6 = OUTMOD_7; //select reset/set (linear actuator)
    TB0CCTL0 &= ~CAP;    //set CAP = 0 for compare mode
    // initialize at 90 deg = ~1.5ms (testing with 1700)
    TB0CCR5 = 2350; //used to be 1700
    TB0CCR6 = 1600;
    TB0CTL |= TBSSEL__SMCLK + MC__UP + ID_0 + TBCLR; //select SMCLK, divider /1
    TB0CCR0 = 20000 - 1;                             // 1000000/50Hz=20000 for 50Hz PWM period - make TB0CCR0 different from 0 to start the timer

    // Timer for stage sequence, 1s period
    TA3CTL = TACLR | TASSEL__SMCLK | MC__UP | ID__8;
    TA3EX0 = TAIDEX_4; // Divider is 8x5, producing 25000Hz from the 1MHz clock
    TA3CCR0 = 38000;   // Count for 1.5s interval

    // Timer for stepper
    TA1CCTL0 &= ~CAP;                                 //compare mode
    TA1CCR0 = 1250;                                   //10ms delay for stepper steps: 125000/1000=125 -> 125*10ms=1250
    TA1CTL |= TASSEL__SMCLK + MC__UP + ID__8 + TACLR; //SMCLK, /8: 1000000/8 = 125000Hz

    // Timer for dispenser servo, 2s period
    TA2CTL = TACLR | TASSEL__SMCLK | MC__UP | ID__8;
    TA2EX0 = TAIDEX_4; // Divider is 8x8?, producing 25000Hz from the 1MHz clock
    TA2CCR0 = 50000;   // Count for 2s interval
}

void init_comms(void)
{
    // Analytics and communication setup

    // *** UART Setup ***

    // Pins
    P4SEL1 &= ~(TX | RX);
    P4SEL0 |= TX | RX;

    // Configure UART mode
    // NOTE: USCI_A0 is used

    UCA0CTLW0 |= UCSWRST;       //eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK; //select SMCLK as source for TX/RX

    //    Baud Rate 115200
    UCA0BRW = 8;        // 1000000/115200 = 8.68
    UCA0MCTLW = 0xD600; // 1000000/115200 - INT(1000000/115200)=0.68
                        // UCBRSx = 0.68 = 0xD6

    UCA0CTLW0 &= ~UCSWRST; // init eUSCI
    UCA0IE |= UCRXIE;      // Enable USCI_A0 RX ISR
    UCA0IFG &= ~UCRXIFG;   //clear interrupt flags

    //  *** ADC Setup ***

    // Pin
    P8SEL1 |= FSR;
    P8SEL0 |= FSR;

    // TODO: Configure ADC
}

void init_unused(void)
{
    // Initializes unused pins to reduce power consumption

    // Pins
    P1DIR |= ~BUZZER;
    P2DIR |= ~(UP | DOWN | ENTER | BACK | LIN_SERVO | CONT_SERVO | IR);
    // P3DIR |= ~(DR_0 | DR_1 | DR_2 | DR_3);
    P4DIR |= ~(SDA | SCL | TX | RX);
    P5DIR |= 0xFF;
    P6DIR |= 0xFF;
    P7DIR |= 0xFF;
    P8DIR |= 0xFF;
    P9DIR |= 0xFF;
    P10DIR |= 0xFF;
}
