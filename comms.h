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
#define hour_field "Hour"
#define minute_field "Minute"
#define pillNames_field "pillNames"
#define pillQuantities_field "pillQuantities"
#define NEW_LINE '\n'





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

// Shared global variables
extern bool finished_rx;
extern char RX_data[buf_size];
#endif /* COMMS_H_ */
