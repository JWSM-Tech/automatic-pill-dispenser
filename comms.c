#include "main.h"
#include "comms.h"

bool finished_rx = false;
char RX_data[buf_size];

char TX_data;
bool error = false;

int buffer_index = 0;

unsigned char temp_hour;
unsigned char temp_minute;
char temp_quantities[8];
char temp_pill_names[8][20];

char *temp_string;
char *stop_array_ptr;
char *array_ptr;
char *end_ptr;

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
    case USCI_NONE:
        break;
    case USCI_UART_UCRXIFG:
    {
        error = false;
        UCA0IFG &= ~UCRXIFG;
        if (UCA0RXBUF != '\n')
        {
            finished_rx = false;
            RX_data[buffer_index++] = UCA0RXBUF;
            new_line_count = 0;
        }
        else
        {
            RX_data[buffer_index] = '\0';
            finished_rx = true;
            buffer_index = 0;
            LPM0_EXIT;
        }
        __no_operation();
        break;
    }
    case USCI_UART_UCTXIFG:
        break;
    case USCI_UART_UCSTTIFG:
        break;
    case USCI_UART_UCTXCPTIFG:
        break;
    }
}

void empty_buffer(unsigned char *buffer, int size)
{
    unsigned int i;
    for (i = 0; i < size; i++)
    {
        buffer[i] = '\0';
    }
}

void add_alarm()
{
    //check schedule to see in what position to add the new alarm
    //struct alarm newAlarm;
    schedule[0].hour = temp_hour;
    schedule[0].minute = temp_minute;
    int i;
    for (i = 0; i < 8; i++)
    {
        strcpy(schedule[0].pill_names[i], temp_pill_names[i]);
        schedule[0].quantities[i] = temp_quantities[i];
    }
}

void check_params(char *RX_data){

    if(strstr(RX_data,hour_field) != NULL){ //Hour is in the string
        temp_string = strstr(RX_data,hour_field);
        char hour_idx = 0;
        array_ptr = strchr(temp_string, ':') + 1;

        stop_array_ptr = strchr(array_ptr, ' ');

        if(stop_array_ptr == NULL){
            error = true;
        }
        temp_hour = 0;

        while(array_ptr[0] != stop_array_ptr[0]){
             if(hour_idx*16)
                temp_hour += array_ptr[0] - '0';
             else
                temp_hour += (array_ptr[0] - '0')*16; //sends hour in Hex BCD
             array_ptr++;
             hour_idx++;
        }

    }

    if((strstr(RX_data,minute_field)) != NULL){ //Minute
        temp_string = strstr(RX_data,minute_field);

        char minute_idx = 0;

        array_ptr = strchr(temp_string, ':') + 1;

        stop_array_ptr = strchr(array_ptr, ' ');

        if(stop_array_ptr == NULL){
            error = true;
        }

        temp_minute = 0;

        while(array_ptr[0] != stop_array_ptr[0]){
             if(minute_idx*16)
                 temp_minute += array_ptr[0] - '0';
             else
                 temp_minute += (array_ptr[0] - '0')*16; //sends minute in Hex BCD
             array_ptr++;
             minute_idx++;
        }
    }

    if((strstr(RX_data,pillNames_field)) != NULL){ //pillNames is in the string
        temp_string = strstr(RX_data,pillNames_field);
        char string_idx = 0;
        char char_idx = 0;
        array_ptr = strchr(temp_string, '[') + 1;
        stop_array_ptr = strchr(temp_string, ']');

        if(stop_array_ptr == NULL){
            error = true;
        }

        while(array_ptr[0] != stop_array_ptr[0]){
            if(array_ptr[0] == ','){
                temp_pill_names[string_idx++][char_idx] = '\0';
                char_idx = 0;
                array_ptr++;
            }
            else{
                 temp_pill_names[string_idx][char_idx++] = array_ptr[0];
                 array_ptr++;
            }
        }
        temp_pill_names[string_idx][char_idx] = '\0';
    }


    if((strstr(RX_data,pillQuantities_field)) != NULL){ //pillQuantities is in the string
        temp_string = strstr(RX_data, pillQuantities_field);
        char char_idx = 0;
        array_ptr = strchr(temp_string, '[') + 1;
        stop_array_ptr = strchr(temp_string, ']');

        if(stop_array_ptr == NULL){
            error = true;
        }

        while(array_ptr[0] != stop_array_ptr[0]){
            if(array_ptr[0] == ','){
                char_idx++;
                array_ptr++;
            }
            else{
                end_ptr = strchr(array_ptr, ',');
                 if(end_ptr != NULL){
                     temp_quantities[char_idx] = strtol(array_ptr, &end_ptr, 10);
                     array_ptr = strchr(array_ptr, ',');
                }
                 else{
                     end_ptr = stop_array_ptr;
                     temp_quantities[char_idx] = strtol(array_ptr, &end_ptr, 10);
                     array_ptr = strchr(array_ptr, ']');
                 }
            }
        }
    }
}
