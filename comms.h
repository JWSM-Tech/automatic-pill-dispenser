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
void check_params(char *RX_data);
//void add_alarm();
void empty_buffer(unsigned char *RX_data, int buffer_index);

// Shared global variables
extern bool finished_rx;
extern char RX_data[buf_size];

#endif /* COMMS_H_ */
