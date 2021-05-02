/*
 * main.h
 *
 *  Created on: Apr 21, 2021
 *      Author: Milton
 */

#ifndef MAIN_H_
#define MAIN_H_
#pragma once

#include <msp430fr6989.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "lcd_control.h"

struct alarm
{
    unsigned char hour;
    unsigned char minute;
    int quantities[8];
    bool taken;
};

//int pill_quantities[8];
//char pill_names[8][15];
struct alarm schedule[8];
extern char pill_names[8][15];
extern char pill_quantities[8];


#define sendAnalyticsParam 1 //send analytics
#define sendNetworkParam 2 // send network
#define sendPillInfoParam 3
#define sendAddReminderParam 4 // send reminder
#define receiveAddReminderParam 4 //receive reminder
#define sendRemoveReminderParam 5 // send reminder
#define receiveRemoveReminderParam 5 //receive reminder
#define sendRefillParam 6 //send refill
#define receiveRefillParam 6 //receive refill
#define sendAddPillParam 7 //send add
#define receiveAddPillParam 7 //receive add
#define sendRemovePillParam 8 //send remove
#define receiveRemovePillParam 8 //receive remove



#endif /* MAIN_H_ */
