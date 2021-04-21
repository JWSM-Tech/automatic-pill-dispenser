/*
 * dispensing-mechanism.h
 *
 *      Author: Jorge Vega
 */

#ifndef DISPENSING_MECHANISM_H_
#define DISPENSING_MECHANISM_H_

#define IR BIT0 //P2.0
#define DISPENSER_SERVO BIT6 //P2.6
#define LINEAR_SERVO BIT7 //P2.7

void dispensing_sequence(int* internalPillContainers);
void make_array_global(int* array);
void move_linear_actuator(int dir);
void rotate_stepper(int dir, int angle);
int get_nearest_container(int* pillContainers);
int get_direction(int index);
int get_distance_to_travel(int index, int dir);
void dispense_pill();
void recalculate_variables(void);
void reset_pill_positions(void);

#endif /* DISPENSING_MECHANISM_H_ */
