/*
 * comms.h
 *
 *  Created on: Apr 21, 2021
 *      Author: Milton
 */

#ifndef COMMS_H_
#define COMMS_H_
#pragma once

#include <stdbool.h>

// Defines

#define buf_size 250
#define TEST_PARSE false
#define MAX_NEW_LINE 3
#define hour_field "Hour"
#define minute_field "Minute"
#define pillNames_field "pillNames"
#define pillQuantities_field "pillQuantities"
#define NEW_LINE '\n'

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





// Functions
void receive_add_reminder();
void receive_remove_reminder();
void receive_refill();
void receive_add_pill();
void receive_remove_pill();


void check_params(char *RX_data);
void empty_buffer(unsigned char *RX_data, int buffer_index);
void send_uart(char param, char index);
char* build_refill_pills();
char* build_network_data();
char* build_analytics();
__int8_t getOldestAlarm();
__int8_t getCurrentAlarm();

// Shared global variables
extern bool finished_rx;
extern char RX_data[buf_size];

extern void add_alarm(unsigned char hour, unsigned char minute, char* quantity);
extern void remove_alarm(unsigned char hour, unsigned char minute, char* quantity);
extern void add_pill(char* pill_name, char pill_quantity);
extern void remove_pill(char* pill_name);
extern void refill(int* pill_quantities);
#endif /* COMMS_H_ */
