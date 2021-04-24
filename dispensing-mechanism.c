#include "main.h"
#include "dispensing-mechanism.h"

/*
 * dispensing-mechanism.c
 *
 *      Author: Jorge Vega
 */

#define IR BIT0              //P2.0
#define DISPENSER_SERVO BIT6 //P2.6
#define LINEAR_SERVO BIT7    //P2.7

#pragma PERSISTENT(LUT)
char LUT[4] = {0x0E, 0x0B, 0x0D, 0x07}; //lookup table for stepper motor inputs

#pragma PERSISTENT(pillMap)
int pillMap[8] = {0, 45, 90, 135, 180, 225, 270, 315}; //mapping pill container positions (index in the array) with their respective angles in the unit circle

int steps = 0;                                    //holds amount of steps the stepper motor should travel when called
int stepperIndex = 0;                             //index to traverse stepper LUT
int currentDirection = 1;                         //direction for stepper, 1 or -1
int pillContainers[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //global variable so it can be accessed by all functions
int origin = 0;                                   //marks the origin for the dispensing sequence
int traveledDistance = 0;                         //used in dispensing sequence
int currentContainer = 0;                         //used in dispensing sequence
int currentDegreesTraveled = 0;                   //used in dispensing sequence
int dispResetFlag = 0;                            //reset flag to indicate if the stepper is moving to reset container positions or to dispense pills
int linearDir = 0;                                //direction the linear actuator moves in, 1 -> up, 0 -> down
int pillsToDispense = 0;                          //keep track of how many pills we have left to dispense for each container during the dispensing sequence
int remainingPills = 0;                           //keep track of total pills to dispense for an alarm
int stage = 1;                                    //stages for the dispensing mechanism, starts at 1
int dispensedFlag = 0;                            //signals if a pill was dispensed or not
int servo_toggle = 0;                             //0 = 0, 1 = 180

void dispensing_sequence(int *internalPillContainers)
{                                                                                        //stage 1 - set up
    make_array_global(internalPillContainers);                                           //save to global variable so it can be accessed by other functions
    currentContainer = get_nearest_container(pillContainers);                            //determine the index of which container is closest to the origin position
    currentDirection = get_direction(currentContainer);                                  //determine the direction in which to rotate
    currentDegreesTraveled = get_distance_to_travel(currentContainer, currentDirection); //determine how many degrees to rotate in the chosen direction
    stage++;                                                                             //go from stage 1 to stage 2
    TA3CCTL0 |= CCIE;                                                                    //enable interrupt for 1.5s timer that traverses dispensing stages
}

void make_array_global(int *array)
{
    int i;
    remainingPills = 0; //reset in case it wasnt 0
    for (i = 0; i < 8; i++)
    {
        pillContainers[i] = array[i]; //populate global pillContainers array with array contents
        if (pillContainers[i] != 0)
        {
            remainingPills += pillContainers[i]; //add to remainingPills for this alarm
        }
    }
}

void move_linear_actuator(int dir)
{
    linearDir = dir; //store direction in global variable for linearAct_ISR
    if (dir == 0)
    {                   //goes down
        TB0CCR6 = 1600; //[1700-100=1600]
    }
    else
    {
        TB0CCR6 = 1800; //[1700+100=1800]
    }

    if (dir == 1)
    {                                                       //linear actuator was told to go up
        pillsToDispense = pillContainers[currentContainer]; //get amount of pills to dispense
    }
    stage++;          //go from stage 3 to stage 4, or 5 to 6 if linearDir == 0
    TA3CCTL0 |= CCIE; //enable interrupt for 1.5s timer that traverses dispensing stages
}

void rotate_stepper(int dir, int angle)
{
    steps = angle / 1.8;
    currentDirection = dir; //set direction to global variable so ISR can access it
    TA1CCTL0 |= CCIE;       //enable timer for delay between steps
}

int get_nearest_container(int *pillContainers)
{
    int minimumDistance = origin;
    int returnIndex = currentContainer;
    int i;

    for (i = 0; i < 8; i++)
    {
        if (pillContainers[i] == 0)
        {
            continue; //skip to next container in the array since there are no pills to dispense
        }
        else
        {
            int distance = origin - pillMap[i]; //calculate distance in degrees from dispensing mechanism to current container
            if (distance < minimumDistance)
            {
                minimumDistance = distance;
                returnIndex = i; //keep track of container's index
            }
        }
    }

    return returnIndex; //return index of container nearest to dispensing mechanism
}

int get_direction(int index)
{
    int angle = pillMap[index]; //get angle from unit circle associated with the pill container at the index
    if (angle > 180)
    {
        return -1; //counterclockwise
    }
    else
    {
        return 1; //clockwise
    }
}

int get_distance_to_travel(int index, int dir)
{
    int degrees;
    if (dir < 0)
    {
        degrees = 360 - pillMap[index] - traveledDistance;
    }
    else
    {
        degrees = pillMap[index] - traveledDistance;
    }
    return degrees;
}

void dispense_pill()
{
    if (pillsToDispense > 0)
    {
        //move the rotating disc inside the pill container to see if the pill falls
        if (servo_toggle == 0)
        {                     //it's in 0 deg
            servo_toggle = 1; //toggle variable
            TB0CCR5 = 800;
        }
        else
        {
            servo_toggle = 0; //toggle variable
            TB0CCR5 = 2350;
        }
        TA2CCTL0 |= CCIE; //enable dispenser ISR that polls dispensed flag every two seconds
    }
    else
    {                     //we're done dispensing
        stage++;          //move from stage 4 to stage 5
        TA3CCTL0 |= CCIE; //enable interrupt for stages ISR
    }
}

void recalculate_variables(void)
{
    //set origin pointer variable to the index of the pill container currently on the origin position, on top of the dispensing mechanism
    origin = pillMap[currentContainer];
    //update the traveledDistance variable to add the distance that was traveled to align the current pill container with the dispenser mechanism
    if (currentDirection < 0)
    { //anti-clockwise
        traveledDistance = traveledDistance + currentDegreesTraveled;
    }
    else
    { //clockwise
        traveledDistance = traveledDistance - currentDegreesTraveled;
    }
    //update pillContainers list for the next iteration of the dispensing sequence to discard from consideration the pill container that was just dispensed
    pillContainers[currentContainer] = 0;
    //check if there are any pills left to dispense
    if (remainingPills > 0)
    {
        currentContainer = get_nearest_container(pillContainers);                            //determine the index of which container is closest to the origin position
        currentDirection = get_direction(currentContainer);                                  //determine the direction in which to rotate
        currentDegreesTraveled = get_distance_to_travel(currentContainer, currentDirection); //determine how many degrees to rotate in the chosen direction
        stage = 2;                                                                           //go again with the updated list to finish dispensing the pills
    }
    else
    {            //no more pills to dispense, reset pill containers to their original positions for the next alarm
        stage++; //go from stage 6 to stage 7 to reset the pill container positions
    }
    TA3CCTL0 |= CCIE; //enable interrupt for stages ISR
}

void reset_pill_positions(void)
{
    dispResetFlag = 1; //to signal we are in reset mode for the stepper ISR
    //reset dispenser servo
    TB0CCR5 = 2350;
    if (origin < 180)
    {
        rotate_stepper(-1, origin);
    }
    else
    {
        int distance = 360 - origin;
        rotate_stepper(1, distance);
    }
}

#pragma vector = TIMER3_A0_VECTOR
__interrupt void stages_ISR(void)
{
    TA3CCTL0 &= ~CCIFG; //clear CCIFG
    TA3CCTL0 &= ~CCIE;  //disable interrupt, gets reactivated in each of the other functions

    switch (stage)
    {
    case 2:
        rotate_stepper(currentDirection, currentDegreesTraveled); //stage 2 - align this iteration�s chosen pill container with the dispensing mechanism
        break;
    case 3:
        move_linear_actuator(1); //stage 3 - raise dispenser servo with linear actuator
        break;
    case 4:
        dispense_pill(); //stage 4 - rotate dispenser servo to dispense pills
        break;
    case 5:
        move_linear_actuator(0); //stage 5 - lower dispenser servo with linear actuator
        break;
    case 6:
        recalculate_variables(); //stage 6 - recalculate sequence variables for next iteration
        break;
    case 7:
        reset_pill_positions(); //stage 7 - reset pill positions for next alarm
        break;
    case 8:
        //reset all variables
        origin = 0;
        traveledDistance = 0;
        currentContainer = 0;
        currentDirection = 0;
        currentDegreesTraveled = 0;
        dispResetFlag = 0;
        servo_toggle = 0;
        stage = 1;
        break;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void stepper_ISR(void)
{
    TA1CCTL0 &= ~CCIFG; //clear CCIFG
    if (steps > 0)
    {
        P3OUT &= ~LUT[stepperIndex];      //clear current output pins for stepper movement
        stepperIndex += currentDirection; //update index for next stepper movement
        //make the LUT iteration circular
        if (stepperIndex > 3)
        {
            stepperIndex = 0;
        }
        else if (stepperIndex < 0)
        {
            stepperIndex = 3;
        }
        steps--;
        P3OUT |= LUT[stepperIndex]; //set output pins to drive stepper movement
    }
    else
    {                      //no more steps to traverse, we're done moving the stepper motor
        TA1CCTL0 &= ~CCIE; //disable compare interrupt
        if (dispResetFlag == 0)
        {
            stage++; //move from stage 2 to stage 3
        }
        else
        {              //if reset == 1, we're done dispensing pills, don't do anything else
            stage = 8; //go to last stage, reset all values
        }
        TA3CCTL0 |= CCIE; //enable interrupt for 1s timer that traverses dispensing stages
    }
}

#pragma vector = TIMER2_A0_VECTOR
__interrupt void dispenser_ISR(void)
{
    TA2CCTL0 &= ~CCIFG; //clear CCIFG
    TA2CCTL0 &= ~CCIE;  //disable dispenser ISR
    if (dispensedFlag == 1)
    {
        pillsToDispense--; //decrement amount of pills to dispense
        remainingPills--;  //decrement total amount of pills to dispense
        dispensedFlag = 0; //reset flag for next pill
    }
    dispense_pill(); //call again to see if we keep going or if we're done
}
