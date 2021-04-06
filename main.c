#include <msp430fr6989.h>
#include "init.h"

/**
 * main.c
 */
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

    // TODO: Initialize LCD


    return 0;
}

//************************************************************************
// ISRs ******************************************************************
//************************************************************************

