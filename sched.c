#include "main.h"
#include "sched.h"
#include "init.h"
#include "dispensing-mechanism.h"

unsigned char alarms_count = 0;
unsigned char alarms_index = 0;
unsigned char menu_index = 0;
char button;

#pragma PERSISTENT(menu)
const char *menu[] = {"1.Set Alarm", "2.Set Time", "3.View Alarms", "4.Add Pill", "5.Settings"};

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
unsigned char day_index = 0;
unsigned char month_index = 0;
unsigned char year_index = 0;

unsigned char day_input = 0;
unsigned char month_input = 0;
unsigned char year_input = 0;
unsigned char hour_input = 0;
unsigned char minute_input = 0;

bool main_menu = true;
bool set_time = false;
bool set_alarm = false;
bool hour_select = false;
bool minute_select = false;
bool buzzer_on = false;
bool view_alarms = false;
bool set_name = false;
bool set_quantities = false;
bool load_pill = false;
bool day_select = false;
bool month_select = false;
bool year_select = false;

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
        button = 1;
        TA0CCR0 = TA0R + 50000;
        TA0CCTL0 |= CCIE;
        break;
    case 8: //ENTER
        button = 2;
        TA0CCR0 = TA0R + 50000;
        TA0CCTL0 |= CCIE;

        break;
    case 4: // UP
        button = 0;
        TA0CCR0 = TA0R + 50000;
        TA0CCTL0 |= CCIE;
        break;

    case 10: // BACK
        button = 3;
        TA0CCR0 = TA0R + 50000;
        TA0CCTL0 |= CCIE;
        break;
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA0_ISR(void)
{
    TA0CCTL0 &= ~CCIE;

    switch(button){
    case 0:
        up_button();
        break;

    case 1:
        down_button();
        break;

    case 2:
        enter_button();
        break;

    case 3:
        back_button();
        break;
    }

}
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA1_ISR(void)
{
    TA0CCR1 += 500;

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
//    P1DIR |= BIT6;
//    P1SEL1 |= BIT6;
//    P1SEL0 |= BIT6;
    TA0CCTL1 = OUTMOD_4 | CCIE;
    TA0CCR1 = 500;
}

void buzzer_off(){
//   P1SEL0 &= ~BIT6;
//   P1SEL1 &= ~BIT6;
//   P1DIR |= BIT6;
    TA0CCTL1 = 0;
   buzzer_on = false;
}

void on(unsigned char index)
{
    clear_display();
    set_cursor(0, 3);
    send_string("Dispenser", 0);
    set_cursor(1, 1);
    send_string(menu[index], 1);


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

void display_calendar(unsigned char day, unsigned char month, unsigned char year){
    clear_display();
    set_cursor(0,0);
    send_string("dd/mm/yyyy",0);
    set_cursor(1,0);
    char *buffer[10];
    BCD2ASC(minute[day], buffer);
    send_string(buffer,0);
    send_string("/",0);
    BCD2ASC(minute[month],buffer);
    send_string(buffer,0);
    send_string("/",0);
    send_string("20",0);
    BCD2ASC(minute[year],buffer);
    send_string(buffer,0);
}

void set_rtc_time(unsigned char day, unsigned char month, unsigned char year, unsigned char hour, unsigned char minute){

       RTCCTL0_H = RTCKEY_H;                     // Unlock RTC
//       RTCCTL0_L = RTCTEVIE | RTCRDYIE | RTCAIE; // enable RTC read ready interrupt
                                                 // enable RTC time event interrupt

       RTCCTL1 = RTCBCD | RTCHOLD | RTCMODE; // RTC enable, BCD mode, RTC hold

       RTCYEAR = year; // Year = 0x2021
       RTCMON = month;     // Month = 0x04 = April
       RTCDAY = day;    // Day = 0x13 = 13
       RTCDOW = 0x03;    // Day of week = 0x02 = tuesday
       RTCHOUR = hour;   // Hour = 0x10
       RTCMIN = minute;    // Minute = 0x00
       RTCSEC = 0x00;    // Seconds = 0x00

       RTCCTL1 &= ~(RTCHOLD); // Start RTC
}

void enter_button(){
    if (buzzer_on)
           { // STOP BUZZER ---- TRY P1.6 AS GPIO AND CLEARING PIN
             buzzer_off();
           }
    if (main_menu)
  {

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

      else if(menu_index == 1){
          //SET TIME MENU

         main_menu = false;
         set_alarm = false;
         set_time = true;
         day_select = true;
         display_calendar(day_index, month_index, year_index);
      }

      else if (menu_index == 2)
      {
          //VIEW ALARMS MENU

          main_menu = false;
          set_alarm = false;
          set_time = false;
          view_alarms = true;
          if (alarms_count == 0)
          {
              display_view_alarms_info();
          }
          else
              display_view_alarms(alarms_index);
      }
  }

   if (set_alarm)
   {

        if (minute_select)
       {                                                         // cursor in hours
           schedule[alarms_count].minute = minute[minute_index]; // user hour input
           minute_select = false;
           hour_index = 0;
           //minute_index = 0;
           on(menu_index);
           main_menu = true;
           set_alarm = false;
           set_time = false;
           alarms_count++;
           //break;
       }
        else if (hour_select)
       {
           set_cursor(0, 0);
           schedule[alarms_count].hour = hour[hour_index];
           hour_select = false;
           minute_select = true;
           set_cursor(0, 3);
           //break;
       }


       else if (set_quantities)
       {
           schedule[alarms_count].quantities[alarms_count] = hour[quantities_index];
           quantities_index = 0;
           set_quantities = false;
           hour_select = true;
           display_time(0, 0);
       }
       else if (set_name)
       {
           strcpy(schedule[alarms_count].pill_names[alarms_count], name[name_index]);
           set_name = false;
           set_quantities = true;
           name_index = 0;
           display_quantity(0);
           //break;
       }


   }

   //        if(view_alarms){
   //
   //        }

   if(set_time){

           if(year_select){
               year_select = false;
               hour_select = true;
               year_input = minute[year_index];
               display_time(0,0);
               //break;
           }
           else if(month_select){ // cursor in hours
              set_cursor(0,6);
              month_select = false;
              year_select = true;
              month_input = minute[month_index]; //using minute array for memory conservation
              //break;
          }
           else if(day_select){
              set_cursor(0,3);
              day_select = false;
              month_select = true;
              day_input = minute[day_index];
             // break;
          }

            if (minute_select){

              minute_input = minute[minute_index];
               minute_select = false;
               hour_index = 0;
               minute_index = 0;
               on(menu_index);
               main_menu = true;
               set_alarm = false;
               set_time = false;
               set_rtc_time(day_input, month_input, year_input, hour_input, minute_input);
              // break;
           }
            if (hour_select){
               //set_cursor(0, 0);
               hour_input = hour[hour_index];
               hour_select = false;
               minute_select = true;
               set_cursor(0, 3);
               //break;
           }

      }


}

void back_button(){
    if (set_alarm)
        {
            if (hour_select)
            {
                hour_select = false;
                set_alarm = false;
                main_menu = true;
                 //menu_index = 0;
                on(menu_index);
            }

            else if (minute_select)
            {
                minute_select = false;
                hour_select = true;
                set_cursor(0, 0);
               // schedule[alarms_index].hour = 0x00;
            }

        }
    if(set_time){
        if(hour_select){
            hour_select = false;
            day_select = true;
            set_cursor(1,0);
            display_calendar(day_index, month_index, year_index);
        }
        if(minute_select){
            minute_select = false;
            hour_select = true;
            set_cursor(1,0);
        }
        if(day_select){
            day_select = false;
            set_time = false;
            main_menu = true;
            on(menu_index);
        }

        if(month_select){
            month_select = false;
            day_select = true;
            set_cursor(1,0);
        }
        if(year_select){
            year_select = false;
            month_select = true;
            set_cursor(1,3);
        }
    }

       if (view_alarms)
        {
            view_alarms = false;
            main_menu = true;
            //menu_index = 0;
            on(menu_index);
        }
}

void up_button(){

    if (main_menu)
    {
       menu_index = (menu_index - 1 + 5) % 5;
       on(menu_index);
    }

    if(set_time)
    {

       if(day_select){ // cursor in minutes
          day_index = (day_index + 1 + 31) % 31;
          display_calendar(day_index, month_index, year_index);
      }

       else if(month_select){ // cursor in hour
           month_index = (month_index + 1 + 12) % 12;
           display_calendar(day_index, month_index, year_index);

       }
       else if(year_select){
           year_index = (year_index + 1 + 60) % 60;
           display_calendar(day_index, month_index, year_index);
       }

       else if(hour_select){
          hour_index = (hour_index + 1 + 24) % 24;
          display_time(hour_index, minute_index);
      }
       else if(minute_select){
          minute_index = (minute_index + 1 + 60) % 60;
          display_time(hour_index, minute_index);
       }

    }

    if (set_alarm)
   {

       if (minute_select)
       { // cursor in minutes
           minute_index = (minute_index + 1 + 60) % 60;
           display_time(hour_index, minute_index);
       }

       else if (hour_select)
       { // cursor in hour
           hour_index = (hour_index + 1 + 25) % 25;
           display_time(hour_index, 0);
       }

       else if (set_name)
       {
           name_index = (name_index + 1 + 8) % 8;
           display_set_name(name_index);
       }

       else if (set_quantities)
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

}

void down_button(){
    if (main_menu)
    {
        menu_index = (menu_index + 1) % 5;
        on(menu_index);
    }

     if(set_time){

        if(day_select){ // cursor in minutes
           day_index = (day_index - 1 + 31) % 31;
           display_calendar(day_index, month_index, year_index);
       }

        else if(month_select){ // cursor in hour
            month_index = (month_index - 1 + 12) % 12;
            display_calendar(day_index, month_index, year_index);

        }
        else if(year_select){
            year_index = (year_index - 1 + 60) % 60;
            display_calendar(day_index, month_index, year_index);
        }
        else if(hour_select){
            hour_index = (hour_index - 1 + 24) % 24;
            display_time(hour_index, minute_index);
        }
        else if(minute_select){
            minute_index = (minute_index - 1 + 60) % 60;
            display_time(hour_index, minute_index);
        }
    }

    if (set_alarm)
    {

        if (minute_select)
        { // cursor in minutes
            minute_index = (minute_index - 1 + 60) % 60;
            display_time(hour_index, minute_index);
        }

        else if (hour_select)
        { // cursor in hour
            hour_index = (hour_index - 1 + 25) % 25;
            display_time(hour_index, minute_index);
        }

        else if (set_name)
        {
            name_index = (name_index - 1 + 8) % 8;
            display_set_name(name_index);
        }

        else if (set_quantities)
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
}

