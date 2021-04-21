#include "main.h"
#include "init.h"
#include "comms.h"

/**
 * main.c
 */

// Global variables

// Current pills in dispenser (string array 8)
// Current pill counts (int array 8)
// Reminders array (determine a limit)

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    // System setup

    init_sched_syst();
    init_disp_mech();
    init_comms();
    init_unused();

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    _bis_SR_register(GIE);

    while (1)
    {
        if (finished_rx)
        {
            check_params(RX_data);
            add_alarm();
            finished_rx = false;
        }
        else
        {
            _bis_SR_register(LPM0_bits + GIE);
        }
    }
}

//************************************************************************
// ISRs ******************************************************************
//************************************************************************
