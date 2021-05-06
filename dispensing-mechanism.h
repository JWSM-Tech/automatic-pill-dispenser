/*
 * dispensing-mechanism.h
 *
 *      Author: Jorge Vega
 */

#ifndef DISPENSING_MECHANISM_H_
#define DISPENSING_MECHANISM_H_

void dispensing_sequence(int *internalPillContainers);
void check_first_nonempty(void);
void make_array_global(int *array);
void move_linear_actuator(int dir);
void stepper_handler(void);
//void dispense_pill(void);
void refill_pills(int *containers);
void done_refilling(void);

extern int dispensedFlag;

#endif /* DISPENSING_MECHANISM_H_ */
