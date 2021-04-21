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

    TA0CCTL1 = OUTMOD_7; // Reset/set output mode
    TA0CTL = (TACLR | TASSEL__SMCLK | ID_0 | MC__UP);
    TA0CCR0 = 0;

    //*** RTC SETUP ***
    // *** BUTTONS SETUP ***

    P2DIR &= ~(UP_BUTTON | DOWN_BUTTON | ENTER_BUTTON | BACK_BUTTON);
    P2IFG &= ~(UP_BUTTON | DOWN_BUTTON | ENTER_BUTTON | BACK_BUTTON);
    P2IES &= ~(UP_BUTTON | DOWN_BUTTON | ENTER_BUTTON | BACK_BUTTON);
    P2IE |= UP_BUTTON | DOWN_BUTTON | ENTER_BUTTON | BACK_BUTTON;

    // Pins

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

    // *** SERVO SETUP ***

    // Pins

    P2DIR |= LIN_SERVO | CONT_SERVO;
    P2OUT &= ~(LIN_SERVO | CONT_SERVO);
    P2SEL1 &= ~(LIN_SERVO | CONT_SERVO);
    P2SEL0 |= LIN_SERVO | CONT_SERVO;

    // Timer

    // TODO: Initialize compare register CCRs
    // Use CCR5 & CCR6 (LIN/CONT)

    TB0CCTL5 = OUTMOD_7; // Reset/set output mode
    TB0CCTL6 = OUTMOD_7; // Reset/set output mode

    TB0CTL = (TBCLR | TBSSEL__SMCLK | ID_0 | MC__UP);
    TB0CCR0 = 0;

    // *** STEPPER SETUP ***

    // Pins

    P3DIR |= DR_0 | DR_1 | DR_2 | DR_3;
    P3OUT &= ~(DR_0 | DR_1 | DR_2 | DR_3);

    /// *** IR SETUP ***

    // Pin

    P2DIR &= ~IR;
    P2IFG &= ~IR;
    P2IES |= IR;
    P2IE &= ~IR;
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
    P2DIR |= ~(UP_BUTTON | DOWN_BUTTON | ENTER_BUTTON | BACK_BUTTON | LIN_SERVO | CONT_SERVO | IR);
    P3DIR |= ~(DR_0 | DR_1 | DR_2 | DR_3);
    P4DIR |= ~(SDA | SCL | TX | RX);
    P5DIR |= 0xFF;
    P6DIR |= 0xFF;
    P7DIR |= 0xFF;
    P8DIR |= 0xFF;
    P9DIR |= 0xFF;
    P10DIR |= 0xFF;
}
