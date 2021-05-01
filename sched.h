/*
 * sched.h
 *
 *  Created on: Apr 21, 2021
 *      Author: milpa
 */

#ifndef SCHED_H_
#define SCHED_H_

#define ALARMS_LENGTH 8
#define PILL_NAME_LENGTH 15
#define MAXIMUM_PILLS 8


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
void add_alarm(unsigned char hour, unsigned char minute, char* quantity);
void remove_alarm(unsigned char hour, unsigned char minute, char* quantity);
void add_pills(char* pill_name, char pill_quantity);
void remove_pill(char* pill_name);
void refill(char* pill_qty);
void display_different_pills_quantity(unsigned char index);
void display_pill_list(char index);
void display_letter(unsigned char index, unsigned char column);
char empty_schedule();
char empty_container();

#endif /* SCHED_H_ */
