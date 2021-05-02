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

int steps = 0; //holds amount of steps the stepper motor should travel when called
int angle = 0; //the current angle the stepper is moving to
char stepperIndex = 0; //index to traverse stepper LUT
char stepperAtOrigin = 0; //0 -> its at origin, 1 -> not at origin
char currentDirection = 1; //direction for stepper, 1 or -1
char pillContainers[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //global variable so it can be accessed by all functions
char currentContainer = 0; //index of the current container being dispensed
char stepperMode = 1; //1 -> dispensing; 2 -> refill
char dispResetFlag = 0; //reset flag to indicate if the stepper is moving to reset container positions or to dispense pills
char pillsToDispense = 0; //keep track of how many pills we have left to dispense for each container during the dispensing sequence
char remainingPills = 0; //keep track of total pills to dispense for an alarm
char stage = 1; //stages for the dispensing mechanism
char dispensedFlag = 0; //signals if a pill was dispensed or not
char dispStage = 1; //stage for dispensing servo sequence

void dispensing_sequence(char *internalPillContainers)
{                                              //stage 1 - set up
    make_array_global(internalPillContainers); //save to global variable so it can be accessed by other functions
    stepperMode = 1;                           //set to dispensing mode
    stage++;                                   //go from stage 1 to stage 2
    TA3CCTL0 |= CCIE;                          //enable interrupt for 1s timer that traverses dispensing stages
}

void check_first_nonempty(void)
{
    char i;
    for (i = 0; i < 8; i++)
    {
        if (pillContainers[i] != 0)
        {
            currentContainer = i; //save current container index
            angle = pillMap[i];   //save angle corresponding to that container
            break;
        }
    }
    stage++;
    TA3CCTL0 |= CCIE; //enable stages isr
}

void make_array_global(char *array)
{
    char i;
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

void move_linear_actuator(char dir)
{
    if (dir == 0)
    {                   //goes down
        TB0CCR6 = 1700;
    }
    else
    {
        TB0CCR6 = 2500;  //goes up
    }

    if (dir == 1)
    {                                                       //linear actuator was told to go up
        pillsToDispense = pillContainers[currentContainer]; //get amount of pills to dispense
    }
    stage++;          //go from stage 3 to stage 4, or 5 to 6 if linearDir == 0
    TA3CCTL0 |= CCIE; //enable interrupt for 1.5s timer that traverses dispensing stages
}

void stepper_handler(void)
{
    if (stepperMode == 1)
    { //dispensing mode
        if ((stepperAtOrigin == 0) && (dispResetFlag == 0))
        {                         //fresh, no reset, havent moved
            currentDirection = 1; //set direction clock wise
            steps = angle / 1.8;
            TA1CCTL0 |= CCIE; //enable stepper ISR
        }
        else if ((stepperAtOrigin == 0) && (dispResetFlag == 1))
        { //ya hicimos reset y volvimos al origen
            stage++;
            TA3CCTL0 |= CCIE; //enable stages ISR
        }
        else if ((stepperAtOrigin == 1) && (dispResetFlag == 1))
        {                                               //ya me movi, wanna move again to reset
            currentDirection = currentDirection * (-1); //opposite direction to go back
            steps = angle / 1.8;
            TA1CCTL0 |= CCIE; //enable stepper ISR
        }
        else if ((stepperAtOrigin == 1) && (dispResetFlag == 0))
        { //me movi first time but still not ready to go back
            stage++;
            TA3CCTL0 |= CCIE; //enable stages ISR
        }
    }
    else if (stepperMode == 2)
    { //refill mode
        if (stepperAtOrigin == 0)
        {                         //fresh, havent moved
            currentDirection = 1; //set direction clock wise
            steps = angle / 1.8;
            TA1CCTL0 |= CCIE; //enable stepper ISR
        }
        else if (stepperAtOrigin == 1)
        {                                               //it has moved
            currentDirection = currentDirection * (-1); //opposite direction to go back
            steps = angle / 1.8;
            TA1CCTL0 |= CCIE; //enable stepper ISR
        }
    }
}

void refill_pills(char *containers)
{
    stepperMode = 2; //set to refill mode
    char i;
    for (i = 0; i < 8; i++)
    {
        if (containers[i] != 0)
        {
            currentContainer = i; //save container's index
            angle = pillMap[i];   //save container's corresponding angle
            break;
        }
    }
    stepper_handler(); //move the steper
}

void done_refilling(void)
{
    stepperMode = 2; //refill mode
    stepper_handler();
}

#pragma vector = TIMER3_A0_VECTOR
__interrupt void stages_ISR(void)
{
    TA3CCTL0 &= ~CCIFG; //clear CCIFG
    TA3CCTL0 &= ~CCIE;  //disable interrupt, gets reactivated in each of the other functions

    switch (stage)
    {
    case 2:
        check_first_nonempty(); //stage 2 - get the next pill container with pills to dispense
        break;
    case 3:
        stepper_handler(); //stage 3 - move stepper to align corresponding pill container with dispenser
        break;
    case 4:
        move_linear_actuator(1); //stage 4 - raise dipsenser servo into pill contaier
        break;
    case 5:
        TA2CCTL0 |= CCIE;  //stage 5 - enable dispenser ISR to move dispenser servo
        break;
    case 6: //stage 6 - delay stage
        stage++;
        TA3CCTL0 |= CCIE; //enable interrupt
        break;
    case 7:
        move_linear_actuator(0); //stage 7
        break;
    case 8: //stage 8
        dispResetFlag = 1;
        pillContainers[currentContainer] = 0; //discard pills from current container
        stepper_handler();
        break;
    case 9:
        if (remainingPills > 0){
            stage = 2;
            TA3CCTL0 |= CCIE; //reenable ISR
        } else {
            stage = 1;
        }
        dispResetFlag = 0;
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
        if (stepperAtOrigin == 0)
        {
            stepperAtOrigin = 1;
        }
        else
        {
            stepperAtOrigin = 0;
        }
        if (stepperMode == 1)
        { //if in dispensing mode
            stepper_handler();
        }
    }
}

#pragma vector = TIMER2_A0_VECTOR
__interrupt void dispenser_ISR(void)
{
    TA2CCTL0 &= ~CCIFG; //clear CCIFG
    switch(dispStage){
    case 1:
        TB0CCR5 = 1000; //align pill holes
        dispStage = 2;
        break;
    case 2:
        TB0CCR5 = 1900; //rotate back
        dispStage = 1;
        if (dispensedFlag == 1){
            pillsToDispense--;
            remainingPills--;
            dispensedFlag = 0;
            if (pillsToDispense == 0){
                TA2CCTL0 &= ~CCIE; //disable dispenser ISR
                stage++;
                TA0CCTL0 |= CCIE; //enable interrupt for stages ISR
            }
        }
        break;
    }
}
