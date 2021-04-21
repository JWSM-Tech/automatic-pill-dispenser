#include <msp430fr6989.h>
#include "init.h"

/**
 * main.c
 */

// Global variables

// Current pills in dispenser (string array 8)
// Current pill counts (int array 8)
// Reminders array (determine a limit)

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // System setup

    init_sched_syst();
    init_disp_mech();
    init_comms();
    init_unused();

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;


    __bis_SR_register(GIE + LPM0_bits);
}

//************************************************************************
// ISRs ******************************************************************
//************************************************************************

