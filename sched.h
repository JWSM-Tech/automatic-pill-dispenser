/*
 * sched.h
 *
 *  Created on: Apr 21, 2021
 *      Author: milpa
 */

#ifndef SCHED_H_
#define SCHED_H_

#define ALARMS_LENGTH 8

// Functions

void buzzer();
void on();
void BCD2ASC(unsigned char src, char *dest);
void display_set_name(unsigned char index);
void display_quantity(unsigned char index);
void display_time(unsigned char hours, unsigned char minutes);
void display_view_alarms_info();
void display_view_alarms(unsigned char index);

#endif /* SCHED_H_ */
