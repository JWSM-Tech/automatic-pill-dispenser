#include "main.h"
#include "sched.h"
#include "init.h"
#include "dispensing-mechanism.h"

unsigned char alarms_count = 0;
unsigned char alarms_index = 0;
unsigned char menu_index = 0;
char pill_count = 0;
char dummy_quantity = 0;
char button;

#pragma PERSISTENT(menu)
const char *menu[] = {"1.Add Alarm", "2.Set Time", "3.View Alarms", "4.Add Pill", "5.Settings"};

#pragma PERSISTENT(name)
//const char *name[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
const char letter[] = {" abcdefghijklmnopqrstuvwxyz"};
char name_buffer[10];

unsigned char letter_index = 0;
unsigned char letter_location_index = 0;
 char quantities_index = 0;
unsigned char different_pills_counter = 0;
unsigned char different_pills_index = 0;

#pragma PERSISTENT(hour)
const unsigned char hour[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24};

#pragma PERSISTENT(minute)
const unsigned char minute[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                                0x38, 0x39, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
                                0x57, 0x58, 0x59};

// Index Variables for BCD Table Lookup
unsigned char hour_index = 0;
unsigned char minute_index = 0;
unsigned char day_index = 0;
unsigned char month_index = 0;
unsigned char year_index = 0;

//Set Time Variables for Set Alarm and Set Time
unsigned char day_input = 0;
unsigned char month_input = 0;
unsigned char year_input = 0;
unsigned char hour_input = 0;
unsigned char minute_input = 0;
int dummy_quantities[8];

int quantities_input = 0;

//Index Variable for Pill List Traversing
char pill_name_index = 0;

//Boolean Variables for Menu States
bool main_menu = true;
bool set_time = false;
bool set_alarm = false;
bool hour_select = false;
bool minute_select = false;
bool buzzer_on = false;
bool view_alarms = false;
bool set_name = false;
bool set_quantities = false;
bool day_select = false;
bool month_select = false;
bool year_select = false;
bool addPill = false;
bool different_pills_select = false;
bool pill_list = false;
bool refilling = false;


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
        down_button();
        break;
    case 8: //ENTER
        enter_button();
        break;
    case 4: // UP
        up_button();
        break;

    case 10: // BACK
        back_button();
        break;
    }
}

//#pragma vector = TIMER0_A0_VECTOR
//__interrupt void TimerA0_ISR(void)
//{
//    TA0CCTL0 &= ~CCIE;
//
//    switch(button){
//    case 0:
//        up_button();
//        break;
//
//    case 1:
//        down_button();
//        break;
//
//    case 2:
//        enter_button();
//        break;
//
//    case 3:
//        back_button();
//        break;
//    }
//
//}
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
    TA0CCTL1 = CCIE;
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
/*Searches for an empty container. An empty container is defined 
by a NULL character. Returns the index of the empty container, else returns -1.
pill_names represents the array of the pills in the device*/
char empty_container(){ 
    unsigned char i;
    for( i = 0; i < 8; i++){
        if(pill_names[i][0] == '\0')
            return i;
    }
    return -1;
}
/*Searches forn an empty schedule. An empty schedule is define 
by having all schedule properties equal to 0. Returns the index of an empty schedule slot, else returns -1.*/
char empty_schedule(){
    char i;
    char j;

    for(i=0;i<ALARMS_LENGTH;i++)
    {
        if(schedule[i].hour == 0x00 && schedule[i].minute == 0x00)
            for(j=0;j<ALARMS_LENGTH;j++)
            {
                if(schedule[i].quantities[j] == 0)
                    return i;
            }

    }
    return -1;
}

// void display_set_name(unsigned char index)
// {
//     clear_display();
//     send_string("Set Pill Name", 0);
//     set_cursor(1, 7);
//     send_string(name[index], 0);
// }

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

    send_string(pill_names[index], 0);
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

void display_letter(unsigned char index, unsigned char column)
{
    //clear_display();
    //send_string("Pill Name",0);
    set_cursor(1,column);
    send_character(letter[index]);
    set_cursor(1,column);
}

void display_pill_list(char index)
{
    clear_display();
    send_string("Available Pills",0);
    set_cursor(1,0);
    send_string("-",0);
    send_string(pill_names[index],0);
}

void display_different_pills_quantity(unsigned char index)
{
    clear_display();
    send_string("Pills per Alarm",0);
    set_cursor(1,7);
    char *buffer[10];
    BCD2ASC(minute[index], buffer);
    send_string(buffer,0);
}

void add_alarm(unsigned char hour, unsigned char minute, char* quantity){
    char empty_slot = empty_schedule();
    char i;
    if(alarms_count <= 7)
    {
    schedule[empty_slot].hour = hour;
    schedule[empty_slot].minute = minute;
    for(i=0;i<8;i++)
    {
        schedule[empty_slot].quantities[i] = quantity[i];
    }
    alarms_count++;
    //return true;
    }
    // else
    //     return false;  
}

void add_pills(char* pill_name, char pill_quantity)
{   if(pill_count<=8)
    {
    char slot = empty_container();
    strcpy(pill_names[slot], pill_name);
    pill_quantities[slot] = pill_quantity;
    pill_count++;
    //return true;
    }
    //return false;
}

void remove_alarm(unsigned char hour, unsigned char minute, char* quantity)
{   char i;
    char j; 
    for(i = 0; i< ALARMS_LENGTH;i++ )
    {
        if(schedule[i].hour == hour && schedule[i].minute == minute)
            for(j = 0; j<ALARMS_LENGTH ; j++)
            {
                if(memcmp(schedule[i].quantities, quantity, sizeof(quantity)) == 0)
                {
                    schedule[i].hour == 0;
                    schedule[i].minute = 0;
                    schedule[i].quantities[j] == 0;
                    alarms_count--;
                }
            }

    }
}

void remove_pill(char* pill_name)
{
    char i;
    char j;
    for(i = 0;i<MAXIMUM_PILLS;i++)
    {   
       for(j = 0; j<PILL_NAME_LENGTH;j++)
       {
           if(strcmp(pill_name, pill_names[i]) == 0)
           {
               pill_names[i][j] = '\0';
               pill_count--;
           }  
       }
    }
    
}

void refill(char* pill_qty)
{
    refilling = true;
    //refill container with integer != 0
    //index of container represents the index in pil_name of pill to refill
    char i;
    for(i=0;i<ALARMS_LENGTH;i++)
    {
        if(pill_qty[i] != 0)
        {
            pill_quantities[i] = pill_qty[i];
            break;
        }
    }
    //refill_pills()
    clear_display();
    set_cursor(0,3);
    send_string("Refilling",0);
    set_cursor(1,0);
    send_string("ENTER if done",0);

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

void enter_button()
{
    if (buzzer_on)
    { 
        buzzer_off();
    }

    else if(refilling)
    {
        on(menu_index); 
    }

    
    else if (main_menu)
  {

      if (menu_index == 0)//SET ALARM MENU
      { 
          main_menu = false;
          set_alarm = true;
          set_time = false;
          hour_select = true;          
          display_time(0,0);
      }

      else if(menu_index == 1) //SET TIME MENU
      {
         main_menu = false;
         set_alarm = false;
         set_time = true;
         day_select = true;
         display_calendar(day_index, month_index, year_index);
      }

      else if (menu_index == 2)//VIEW ALARMS MENU
      {
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
      else if(menu_index == 3)// ADD PILL MENU
      {  
          if(empty_container() == -1)
          {
              //display full condition menu
          }
          else
          {
          clear_display();
          addPill = true;
          main_menu = false;
          set_name = true;
          send_string("Pill Name",0);
          set_cursor(1,0);
          display_letter(letter_index, letter_location_index);
          }
      }
  }
  else if(addPill){ //ADD PILL MENU

          if(set_quantities) // user has selected quantity of pills added to a container
          {
              set_quantities = false;
              main_menu = true;
              addPill = false;
              dummy_quantity = quantities_index;
              quantities_index = 0;
              add_pills(name_buffer,dummy_quantity);
              on(menu_index);
          }
          
          if(set_name)
          {
            if(letter_location_index <= 10)
            { // if there is space available for writting name
                name_buffer[letter_location_index] = letter[letter_index];
                letter_location_index++;
                letter_index = 0;
                set_cursor(1,letter_location_index);
            }
            else
            { // not enough space, writting name is done
                set_name = false;
                set_quantities = true;
                letter_location_index = 0;
                letter_index = 0;
                //strcpy(pill_names[empty_index], name_buffer);
                display_quantity(quantities_index);
            }  
          }
           
      }

    else if (set_alarm)
   {

        if (minute_select) // SELECCION DE MINUTO DE ALARMA
       {                                                         
           minute_input = minute[minute_index];
           minute_select = false;
           different_pills_select = true;
           //display select diferent pills menu
           display_different_pills_quantity(different_pills_index); // update counter in up/down button

                     
       }
        else if (hour_select) // SELECCION DE HORA DE ALARMA
       {
           //set_cursor(0, 0);
           //schedule[alarms_count].hour = hour[hour_index]; // hour_input = hour[hour_index], llamar add_alarm cuando el minuto tambien este set
           hour_input = hour[hour_index];
           hour_select = false;
           minute_select = true;
           //set_cursor(0, 3);
           
       }
       else if(different_pills_select) // SELECCION DE # DE DIFERENTES PASTILLAS A DISPENSAR EN LA ALARMA
       {
           different_pills_select = false;
           pill_list = true;
           display_pill_list(pill_name_index);
           different_pills_counter = different_pills_index;

       }
       
       else if(pill_list) // SELECCION DE CONTAINER(NOMBRE DE PASTILLA) A AÑADIR A LA ALARMA
       {  
               //display_pill_list(pill_name_index);
               set_quantities = true;
               pill_list = false;
               display_quantity(quantities_index);    // ? pasar aqui el index del pill seleccionado   
       }

       else if (set_quantities)
       {
           different_pills_counter--;
           if(different_pills_counter > 0)
           {
               dummy_quantities[pill_name_index] = quantities_index;
               pill_list = true;
               set_quantities = false;
               pill_name_index = 0;
               quantities_index = 0;
               display_pill_list(pill_name_index);
           }
           else
           {
               set_quantities = false;
               main_menu = true;
               dummy_quantities[pill_name_index] = quantities_index;
               quantities_index = 0;
               hour_index = 0;
                minute_index = 0;
               on(menu_index);
               add_alarm(hour_input, minute_input, dummy_quantities);
           }
       }


   }
   //        if(view_alarms){
   //
   //        }

    else if(set_time){

           if(year_select){
               year_select = false;
               hour_select = true;
               year_input = minute[year_index];
               display_time(0,0);
               
           }
           else if(month_select){ // cursor in hours
              set_cursor(0,6);
              month_select = false;
              year_select = true;
              month_input = minute[month_index]; //using minute array for memory conservation
              
          }
           else if(day_select){
              set_cursor(0,3);
              day_select = false;
              month_select = true;
              day_input = minute[day_index];
             
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
              
           }
            if (hour_select){
               //set_cursor(0, 0);
               hour_input = hour[hour_index];
               hour_select = false;
               minute_select = true;
               set_cursor(0, 3);
               
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

        else if(different_pills_select)
        {
            different_pills_select = false;
            minute_select = true;
            display_time(hour_index, minute_index);
        }

        else if(pill_list)
        {
            pill_list = false;
            different_pills_select = true;
            display_different_pills_quantity(different_pills_index);
        }


        //ADD BACK FUNCTIONALITY FOR PILL_LIST, DIFFERENT_PILL_NUMBER, SET_QUANTITY

    }
    if(set_time)
    {
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

    if(addPill)
    {
        if(set_quantities)
        {
            set_quantities = false;
            set_name = true;
            clear_display();
            send_string("Pill Name",0);
            set_cursor(1,0);
            display_letter(letter_index, letter_location_index);
            
        }
        else if(set_name)
        {
            set_name = false;
            main_menu = true;
            on(menu_index);
        }        
    }
}

void up_button(){

    if (main_menu)
    {
       menu_index = (menu_index - 1 + 5) % 5;
       on(menu_index);
    }

    else if(set_time)
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

    else if (set_alarm)
   {

       if (minute_select)
       { // cursor in minutes
           minute_index = (minute_index + 1 + 60) % 60;
           display_time(hour_index, minute_index);
       }

       else if (hour_select)
       { // cursor in hour
           hour_index = (hour_index + 1 + 25) % 25;
           display_time(hour_index, minute_index);
       }

       else if (set_quantities)
       {
           quantities_index = (quantities_index + 1 + 25) % 25;
           display_quantity(quantities_index);
       }

       else if(pill_list)
       {
           pill_name_index = (pill_name_index - 1 + 8) % 8;
           display_pill_list(pill_name_index);
       }

      else if(different_pills_select){
           different_pills_index = (different_pills_index + 1 + 8) % 8;
           display_different_pills_quantity(different_pills_index);
       }
   }

    else if (view_alarms)
   {
       alarms_index = (alarms_index - 1 + alarms_count) % alarms_count;
       display_view_alarms(alarms_index);
   }

    else if(addPill){
       if(set_name){            
            letter_index = (letter_index + 1) % 27;
            display_letter(letter_index,letter_location_index);    
       }
        if(set_quantities){
            quantities_index = (quantities_index + 1) % 25;
            display_quantity(quantities_index);
        }
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

        else if (set_quantities)
        {
            quantities_index = (quantities_index - 1 + 25) % 25;
            display_quantity(quantities_index);
        }

       else if(pill_list)
       {
           pill_name_index = (pill_name_index + 1 + 8) % 8;
           display_pill_list(pill_name_index);
       }

       else if(different_pills_select){
           different_pills_index = (different_pills_index - 1 + 8) % 8;
           display_different_pills_quantity(different_pills_index);
       }
    }

     if (view_alarms)
    {
        alarms_index = (alarms_index + 1) % alarms_count;
        display_view_alarms(alarms_index);
    }

    if(addPill){
       if(set_name){
           letter_index = (letter_index - 1 + 27) % 27;
           display_letter(letter_index, letter_location_index);
       }
       if(set_quantities){
           quantities_index = (quantities_index - 1 + 25) % 25;
           display_quantity(quantities_index);
       }
   }
}

