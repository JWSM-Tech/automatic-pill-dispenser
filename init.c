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
    RTCCTL0_H = RTCKEY_H;            // Unlock RTC
    RTCCTL0_L = RTCTEVIE | RTCRDYIE; // enable RTC read ready interrupt
                                     // enable RTC time event interrupt

    RTCCTL1 = RTCBCD | RTCHOLD | RTCMODE; // RTC enable, BCD mode, RTC hold

    // Initial date

    RTCYEAR = 0x2021; // Year = 0x2010
    RTCMON = 0x4;     // Month = 0x04 = April
    RTCDAY = 0x06;    // Day = 0x05 = 5th
    RTCDOW = 0x02;    // Day of week = 0x01 = Monday
    RTCHOUR = 0x17;   // Hour = 0x10
    RTCMIN = 0x13;    // Minute = 0x32
    RTCSEC = 0x00;    // Seconds = 0x45

    // Alarm date

    RTCADOWDAY = 0x2; // RTC Day of week alarm = 0x2
    RTCADAY = 0x20;   // RTC Day Alarm = 0x20
    RTCAHOUR = 0x10;  // RTC Hour Alarm
    RTCAMIN = 0x23;   // RTC Minute Alarm

    RTCCTL1 &= ~(RTCHOLD); // Start RTC

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
    UCB1CTL1 |= UCSWRST;                           // put eUSCI_B in reset state
    UCB1CTLW0 |= UCMODE_3 + UCMST + UCSSEL__SMCLK; // I2C master mode
    UCB1BRW = 0x10;                                // baud rate = SMCLK / 10 = ~100kHz
    UCB1CTLW1 = UCASTP_2;                          // automatic STOP assertion
    UCB1TBCNT = 0x02;                              // TX 7 bytes of data
    UCB1I2CSA = SLAVE_ADDR;                        // address slave is 12hex
    UCB1CTL1 &= ~UCSWRST;                          // eUSCI_B in operational state
    UCB1IE |= UCTXIE;                              // enable TX-interrupt
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

    // TODO: Set proper baud rate

    UCA0CTLW0 = UCSWRST;       // Put eUSCI in reset
    UCA0CTL1 |= UCSSEL__SMCLK; // CLK = SMCLK
    UCA0BR0 = 8;               // 1000000/115200 = 8.68
    UCA0MCTLW = 0xD600;        // 1000000/115200 - INT(1000000/115200)=0.68
                               // UCBRSx value = 0xD6 (See UG)
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST; // release from reset
    UCA0IE |= UCRXIE;     // Enable USCI_A0 RX interrupt

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
