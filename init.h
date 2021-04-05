/*
 * init.h
 *
 *  Created on: Apr 5, 2021
 *      Author: Milton Pagan
 */

/*
 * Contains constant definitions and initialization code
 */

#ifndef INIT_H_
#define INIT_H_
#pragma once

//***********************************************************************************
// Definitions ******************************************************************
//***********************************************************************************

#define SDA BIT0        // SDA Pin
#define SCL BIT1        // SCL Pin

#define SLAVE_ADDR 0x00 // LCD I2C slave address

//************************************************************************
// Functions *************************************************************
//************************************************************************

void init_sched_syst() {
    // Scheduling system setup

    // I2C Setup
    // NOTE: eUSCI1_B is used

    // Pins
    P4SEL1 |= SDA | SCL;
    P4SEL0 &= ~SDA & ~SCL;

    // I2C registers
    UCB1CTLW0 = UCSWRST;                      // Enable SW reset
    UCB1CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK | UCSYNC; // I2C master mode, SMCLK
    UCB1BRW = 10;                            // fSCL = SMCLK/10 = ~100kHz
    UCB1I2CSA = SLAVE_ADDR;                   // Slave Address
    UCB1CTLW0 &= ~UCSWRST;                    // Clear SW reset, resume operation
    UCB1IE |= UCNACKIE;
}

void init_disp_mech() {
    // Dispensing mechanism setup

}

void init_comms() {
    // Analytics and communication setup

    // UART Setup
}

void init_unused() {

}


#endif /* INIT_H_ */
