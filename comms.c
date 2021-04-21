#include "main.h"
#include "comms.h"

char TEST_PARSE_STRING[buf_size] = "Hour:19 Minute:15 pillNames:[wept,Milton,did,op,one,bai,bee,boy] pillQuantities:[49,12,81,26,57,105,201,304]";

bool finished_rx = false;
char RX_data[buf_size];
int new_line_count = 0;

char TX_data;
bool error = false;

int buffer_index = 0;

unsigned char temp_hour;
unsigned char temp_minute;
int temp_quantities[8];
char temp_pill_names[8][20];

char temp_string[200];
char stop_array_ptr[200];
char array_ptr[200];

char *a_ptr;
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
    schedule.hour = temp_hour;
    schedule.minute = temp_minute;
    int i;
    for (i = 0; i < 8; i++)
    {
        strcpy(schedule.pill_names[i], temp_pill_names[i]);
        schedule.quantities[i] = temp_quantities[i];
    }
}

void check_params(char *RX_data)
{
    //char *strstr(const char *haystack, const char *needle)

    if (strstr(RX_data, hour_field) != NULL)
    { //Hour is in the string
        strcpy(temp_string, (strstr(RX_data, hour_field)));
        int hour_idx = 0;
        strcpy(array_ptr, strchr(temp_string, ':'));
        (a_ptr = array_ptr + 1);

        strcpy(stop_array_ptr, strchr(array_ptr, ' '));

        if (stop_array_ptr == NULL)
        {
            error = true;
        }
        temp_hour = 0;

        while (a_ptr[0] != stop_array_ptr[0])
        {
            //             temp_hour += array_ptr[hour_idx]*(hour_idx*16);
            if (hour_idx * 16)
                temp_hour += a_ptr[0] - '0';
            else
                temp_hour += (a_ptr[0] - '0') * 16; //sends hour in Hex BCD
            a_ptr++;
            hour_idx++;
        }
    }

    if ((strstr(RX_data, minute_field)) != NULL)
    { //Minute
        strcpy(temp_string, (strstr(RX_data, minute_field)));
        int minute_idx = 0;
        strcpy(array_ptr, strchr(temp_string, ':'));
        a_ptr = array_ptr + 1;
        strcpy(stop_array_ptr, strchr(array_ptr, ' '));

        if (stop_array_ptr == NULL)
        {
            error = true;
        }
        temp_minute = 0;

        while (a_ptr[0] != stop_array_ptr[0])
        {
            if (minute_idx * 16)
                temp_minute += a_ptr[0] - '0';
            else
                temp_minute += (a_ptr[0] - '0') * 16; //sends minute in Hex BCD
            a_ptr++;
            minute_idx++;
        }
    }

    if ((strstr(RX_data, pillNames_field)) != NULL)
    { //pillNames is in the string
        strcpy(temp_string, (strstr(RX_data, pillNames_field)));
        int string_idx = 0;
        int char_idx = 0;
        strcpy(array_ptr, strchr(temp_string, '['));
        a_ptr = array_ptr + 1;
        int ptr_idx = 0;
        strcpy(stop_array_ptr, strchr(temp_string, ']'));

        if (stop_array_ptr == NULL)
        {
            error = true;
        }

        while (a_ptr[0] != stop_array_ptr[0])
        {
            if (a_ptr[0] == ',')
            {
                temp_pill_names[string_idx++][char_idx] = '\0';
                char_idx = 0;
                a_ptr++;
            }
            else
            {
                temp_pill_names[string_idx][char_idx++] = a_ptr[0];
                a_ptr++;
            }
        }
        temp_pill_names[string_idx][char_idx] = '\0';
    }

    if ((strstr(RX_data, pillQuantities_field)) != NULL)
    { //pillQuantities is in the string
        strcpy(temp_string, (strstr(RX_data, pillQuantities_field)));
        int char_idx = 0;
        int quantity_idx = 0;
        strcpy(array_ptr, strchr(temp_string, '['));
        a_ptr = array_ptr + 1;
        strcpy(stop_array_ptr, strchr(temp_string, ']'));

        if (stop_array_ptr == NULL)
        {
            error = true;
        }

        while (a_ptr[0] != stop_array_ptr[0])
        {
            if (a_ptr[0] == ',')
            {
                char_idx++;
                a_ptr++;
                quantity_idx = 0;
            }
            else
            {
                end_ptr = strchr(a_ptr, ',');
                if (end_ptr != NULL)
                {
                    temp_quantities[char_idx] = strtol(a_ptr, &end_ptr, 10);
                    a_ptr = strchr(a_ptr, ',');
                }
                else
                {
                    end_ptr = stop_array_ptr;
                    temp_quantities[char_idx] = strtol(a_ptr, &end_ptr, 10);
                    a_ptr = strchr(a_ptr, ']');
                }
            }
        }
    }
}