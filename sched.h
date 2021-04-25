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
void on(unsigned char index);
void BCD2ASC(unsigned char src, char *dest);
void display_set_name(unsigned char index);
void display_quantity(unsigned char index);
void display_time(unsigned char hours, unsigned char minutes);
void display_view_alarms_info();
void display_view_alarms(unsigned char index);
void buzzer_off();
void display_calendar(unsigned char day, unsigned char month, unsigned char year);
void set_rtc_time(unsigned char day, unsigned char month, unsigned char year, unsigned char hour, unsigned char minute);
void enter_button();
void up_button();
void down_button();
void back_button();

#endif /* SCHED_H_ */
