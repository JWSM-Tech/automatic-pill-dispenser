#include "main.h"
#include "init.h"
#include "comms.h"
#include "dispensing-mechanism.h"
#include "sched.h"

/**
 * main.c
 */

// Global variables

// Current pills in dispenser (string array 8)
// Current pill counts (int array 8)
// Reminders array (determine a limit)

char pill_names[8][15];
char pill_quantities[8];

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // System setup

    init_sched_syst();
    init_disp_mech();
    init_comms();

    display_init();
    on();
    _bis_SR_register(GIE);

    while (1)
    {
        if (finished_rx)
        {
            check_params(RX_data);
            finished_rx = false;
        }
        else
        {
            _bis_SR_register(LPM0_bits + GIE);
        }
    }
}
