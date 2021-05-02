/*
 * dispensing-mechanism.h
 *
 *      Author: Jorge Vega
 */

#ifndef DISPENSING_MECHANISM_H_
#define DISPENSING_MECHANISM_H_

void dispensing_sequence(char *internalPillContainers);
void check_first_nonempty(void);
void make_array_global(char *array);
void move_linear_actuator(char dir);
void stepper_handler(void);
void refill_pills(char *containers);
void done_refilling(void);

extern char dispensedFlag;

#endif /* DISPENSING_MECHANISM_H_ */
