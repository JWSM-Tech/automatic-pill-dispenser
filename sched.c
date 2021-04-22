#include "main.h"
#include "sched.h"
#include "init.h"
#include "dispensing-mechanism.h"

unsigned char alarms_count = 0;
unsigned char alarms_index = 0;
unsigned char menu_index = 0;

#pragma PERSISTENT(menu)
const char *menu[] = {"1.Set Alarm", "2.Set Time", "3.View Alarms", "4. Edit Alarms", "5.Settings"};

#pragma PERSISTENT(name)
const char *name[] = {"A", "B", "C", "D", "E", "F", "G", "H"};

unsigned char name_index = 0;
unsigned char quantities_index = 0;

#pragma PERSISTENT(hour)
const unsigned char hour[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24};

#pragma PERSISTENT(minute)
const unsigned char minute[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                                0x38, 0x39, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
                                0x57, 0x58, 0x59};

unsigned char hour_index = 0;
unsigned char minute_index = 0;

bool main_menu = true;
bool set_time = false;
bool set_alarm = false;
bool hour_select = false;
bool minute_select = false;
bool buzzer_on = false;
bool view_alarms = false;
bool set_name = false;
bool set_quantities = false;

// ISRs
#pragma vector = PORT2_VECTOR
__interrupt void port2_handler(void)
{

    switch (P2IV)
    {
    case 2:
        P2IFG &= ~IR;      //clear interrupt flag
        dispensedFlag = 1; //pill was dispensed
        break;

    case 6: //DOWN
        __delay_cycles(250000);
        if (main_menu)
        {
            clear_display();
            set_cursor(0, 3);
            send_string("Dispenser", 0);
            menu_index = (menu_index + 1) % 5;
            set_cursor(1, 1);
            send_string(menu[menu_index], 1);
        }
        //        if(set_time){
        //            if(enter_count == 1){ // cursor in minutes
        //               minute_index = (minute_index - 1 + 60) % 60;
        //               display_time(hour_index,minute_index);
        //           }
        //            if(enter_count == 0){ // cursor in hour
        //                hour_index = (hour_index - 1 + 13) % 13;
        //                display_time(hour_index,0);
        //
        //            }
        //        }

        if (set_alarm)
        {

            if (minute_select)
            { // cursor in minutes
                minute_index = (minute_index - 1 + 60) % 60;
                display_time(hour_index, minute_index);
            }

            if (hour_select)
            { // cursor in hour
                hour_index = (hour_index - 1 + 25) % 25;
                display_time(hour_index, minute_index);
            }

            if (set_name)
            {
                name_index = (name_index - 1 + 8) % 8;
                display_set_name(name_index);
            }

            if (set_quantities)
            {
                quantities_index = (quantities_index - 1 + 25) % 25;
                display_quantity(quantities_index);
            }
        }

        if (view_alarms)
        {
            alarms_index = (alarms_index + 1) % alarms_count;
            display_view_alarms(alarms_index);
        }
        break;
    case 8: //ENTER
        __delay_cycles(250000);
        if (buzzer_on)
        { // STOP BUZZER ---- TRY P1.6 AS GPIO AND CLEARING PIN
            P1SEL0 &= ~BIT6;
            P1SEL1 &= ~BIT6;
            P1DIR |= BIT6;
            P1OUT &= ~BIT6;
            buzzer_on = false;
            break;
        }

        if (set_alarm)
        {

            if (hour_select)
            {
                set_cursor(0, 0);
                schedule[alarms_count].hour = hour[hour_index];
                hour_select = false;
                minute_select = true;
                set_cursor(0, 3);
                break;
            }
            if (minute_select)
            {                                                         // cursor in hours
                schedule[alarms_count].minute = minute[minute_index]; // user hour input
                minute_select = false;
                hour_index = 0;
                minute_index = 0;
                on();
                main_menu = true;
                set_alarm = false;
                set_time = false;
                alarms_count++;
                break;
            }

            if (set_name)
            {
                strcpy(schedule[alarms_count].pill_names[alarms_count], name[name_index]);
                set_name = false;
                set_quantities = true;
                name_index = 0;
                display_quantity(0);
                break;
            }

            if (set_quantities)
            {
                schedule[alarms_count].quantities[alarms_count] = hour[quantities_index];
                quantities_index = 0;
                set_quantities = false;
                hour_select = true;
                display_time(0, 0);
            }
        }

        //        if(view_alarms){
        //
        //        }

        //        if(set_time){
        //
        //            if(enter_count == 1){
        //               set_cursor(0,3);
        //               minute_input = minute[minute_index];
        //               enter_count = 0;
        //               set_time = 0;
        //               main_menu = 1;
        //               on1();
        //               break;
        //           }
        //           if(enter_count == 0){ // cursor in hours
        //               hour_input = hour[hour_index]; // user hour input
        //               enter_count++;
        //               set_cursor(0,3);
        //           }
        //
        //       }

        if (main_menu)
        {
            // enter_count = 0;
            if (menu_index == 0)
            { // set alarm menu
                main_menu = false;
                set_time = false;
                set_alarm = true;
                set_name = true;
                display_set_name(0);
                //hour_select = true;
                //display_set_name
                //display_time(0,0);
            }
            //            if(menu_index == 1){
            //               main_menu = false;
            //               set_alarm = false;
            //               //set_time = true;
            //               hour_select = true;
            //               //display_time(0,0);
            //               //set_cursor(0,0);
            //            }

            if (menu_index == 2)
            {
                main_menu = false;
                set_alarm = false;
                set_time = false;
                view_alarms = true;
                if (alarms_count == 0)
                {
                    display_view_alarms_info();
                    break;
                }

                display_view_alarms(alarms_index);
            }
        }

        break;
    case 4: // UP
        __delay_cycles(250000);
        if (main_menu)
        {
            clear_display();
            set_cursor(0, 3);
            send_string("Dispenser", 0);
            menu_index = (menu_index - 1 + 5) % 5;
            set_cursor(1, 1);
            send_string(menu[menu_index], 1);
        }
        //        if(set_time){
        //
        //            if(minute_select){ // cursor in minutes
        //               minute_index = (minute_index + 1) % 60;
        //               display_time(hour_index, minute_index);
        //           }
        //           if(hour_select){ // cursor in hour
        //               hour_index = (hour_index + 1) % 25;
        //               display_time(hour_index,0);
        //           }
        //
        //       }

        if (set_alarm)
        {

            if (minute_select)
            { // cursor in minutes
                minute_index = (minute_index + 1 + 60) % 60;
                display_time(hour_index, minute_index);
            }

            if (hour_select)
            { // cursor in hour
                hour_index = (hour_index + 1 + 25) % 25;
                display_time(hour_index, 0);
            }

            if (set_name)
            {
                name_index = (name_index + 1 + 8) % 8;
                display_set_name(name_index);
            }

            if (set_quantities)
            {
                quantities_index = (quantities_index + 1 + 25) % 25;
                display_quantity(quantities_index);
            }
        }
        if (view_alarms)
        {
            alarms_index = (alarms_index - 1 + alarms_count) % alarms_count;
            display_view_alarms(alarms_index);
        }

        break;

    case 10: // BACK
        __delay_cycles(250000);
        if (set_alarm)
        {
            if (hour_select)
            {
                hour_select = false;
                set_alarm = false;
                main_menu = true;

                menu_index = 0;
                on();
            }

            if (minute_select)
            {

                minute_select = false;
                hour_select = true;
                set_cursor(0, 0);
                schedule[alarms_index].hour = 0x00;
            }
        }

        if (view_alarms)
        {
            view_alarms = false;
            main_menu = true;
            menu_index = 0;
            on();
        }
        break;
    }
}

#pragma vector = RTC_VECTOR
__interrupt void RTC_ISR(void)
{
    switch (__even_in_range(RTCIV, RTCIV_RT1PSIFG))
    {
    case RTCIV_NONE:
        break; // No interrupts
    case RTCIV_RTCOFIFG:
        break;            // RTCOFIFG
    case RTCIV_RTCRDYIFG: // RTCRDYIFG
                          // P1OUT ^= 0x01;                  // Toggles P1.0 every second

        break;
    case RTCIV_RTCTEVIFG: // RTCEVIFG Check if an alarm is supposed to be set
    {
        int i;
        for (i = 0; i < ALARMS_LENGTH; i++)
        {

            if (schedule[i].hour == RTCHOUR && schedule[i].minute == RTCMIN)
            {
                buzzer();
                dispensing_sequence(schedule[i].quantities);
                //display corresponding alarm
            }
        }

        __no_operation(); // Interrupts every minute
        break;
    }
    case RTCIV_RTCAIFG:
        break; // RTCAIFG
    case RTCIV_RT0PSIFG:
        break; // RT0PSIFG
    case RTCIV_RT1PSIFG:
        break; // RT1PSIFG
    default:
        break;
    }
}

// Functions

void buzzer()
{
    buzzer_on = true;
    P1DIR |= BIT6;
    P1SEL1 |= BIT6;
    P1SEL0 |= BIT6;
    TA0CCR0 = 1000 - 1;
    TA0CCR1 = 999;
}

void on()
{
    clear_display();
    set_cursor(0, 3);
    send_string("Dispenser", 0);
    set_cursor(1, 1);
    send_string(menu[0], 1);
}

void BCD2ASC(unsigned char src, char *dest)
{
    static const char outputs[] = "0123456789ABCDEF";
    *dest++ = outputs[src >> 4];
    *dest++ = outputs[src & 0xf];
    *dest = '\0';
}

void display_set_name(unsigned char index)
{
    clear_display();
    send_string("Set Pill Name", 0);
    set_cursor(1, 7);
    send_string(name[index], 0);
}

void display_quantity(unsigned char index)
{
    clear_display();
    send_string("Set Quantity", 0);
    set_cursor(1, 7);
    char *buffer[10];
    BCD2ASC(hour[index], buffer);
    send_string(buffer, 0);
}

void display_time(unsigned char hours, unsigned char minutes)
{

    clear_display();
    set_cursor(0, 3);
    send_string("Hr", 0);
    set_cursor(0, 6);
    send_string("Mn", 0);
    char *buffer[10];
    BCD2ASC(hours[hour], buffer);
    set_cursor(1, 3);
    send_string(buffer, 0);
    send_string(":", 0);
    BCD2ASC(minute[minutes], buffer);
    send_string(buffer, 0);
}

void display_view_alarms_info()
{
    clear_display();
    send_string("Name", 0);
    set_cursor(0, 6);
    send_string("Qty", 0);
    set_cursor(0, 11);
    send_string("Time", 0);
}

void display_view_alarms(unsigned char index)
{

    clear_display();
    char *buffer[10];
    display_view_alarms_info();
    set_cursor(1, 0);

    send_string(schedule[index].pill_names[index], 0);
    set_cursor(1, 7);
    ltoa((long)schedule[index].quantities[index], buffer, 10);
    send_string(buffer, 0);
    set_cursor(1, 10);
    BCD2ASC(schedule[index].hour, buffer);
    send_string(buffer, 0);
    send_string(":", 0);
    BCD2ASC(schedule[index].minute, buffer);
    send_string(buffer, 0);
}
