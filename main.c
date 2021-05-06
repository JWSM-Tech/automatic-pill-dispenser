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

// TODO: REMOVE THIS
#define DEBUG 0

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
    on(0);

    _bis_SR_register(GIE);

    if (DEBUG)
    {
        int test[8] = {0, 0, 1, 0, 2, 0, 0, 3};
        dispensing_sequence(test);
    }

    else
    {
        while (1)
        {
            if (finished_rx)
            {
                check_params(RX_data);
                finished_rx = false;
            }
            else if(time_elapsed && !schedule[get_current_alarm()].taken)
            {
                ADC12CTL0 |= ADC12ENC | ADC12SC; // Enable and Start conversion
                __bis_SR_register(LPM0_bits + GIE); // LPM0, ADC12_ISR will force exit
            }
            else
            {
                _bis_SR_register(LPM0_bits + GIE);
            }
        }
    }
}
