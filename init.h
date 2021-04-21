/*
 * init.h
 *
 *  Created on: Apr 5, 2021
 *  Author: Milton Pagan
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

// *** Scheduling System ***

// Port 4
#define SDA BIT0 // SDA Pin
#define SCL BIT1 // SCL Pin

// Port 1
#define BUZZER BIT6 // Buzzer pin

// Port 2
#define UP_BUTTON BIT1
#define DOWN_BUTTON BIT2
#define ENTER_BUTTON BIT3
#define BACK_BUTTON BIT4

#define SLAVE_ADDR 0x27 // LCD I2C slave address

// *** Dispensing Mechanism ***

// Port 2
#define LIN_SERVO BIT6
#define CONT_SERVO BIT7

// Stepper driver control lines -> Port 3
#define DR_0 BIT0
#define DR_1 BIT1
#define DR_2 BIT2
#define DR_3 BIT3

// Port 2
#define IR BIT0
#define DISPENSER_SERVO BIT6 //P2.6
#define LINEAR_SERVO BIT7 //P2.7

// ** Analytics and Communitation **

// Port 4
#define TX BIT2
#define RX BIT3

// Port 8
#define FSR BIT4

//************************************************************************
// Functions *************************************************************
//************************************************************************

void init_clock(void);
void init_sched_syst(void);
void init_disp_mech(void);
void init_comms(void);
void init_unused(void);

#endif /* INIT_H_ */
