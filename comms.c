#include "main.h"
#include "comms.h"

bool finished_rx = false;
char RX_data[buf_size];

char TXSendBuffer[buf_size];
bool error = false;

char *SSID = "";
char *PASSWORD = "";

char pill_names[8][15]; //TODO: replace with refill pill_names variable
char quantities[8]; //TODO: replace with refill quantities variable

int buffer_index = 0;

unsigned char temp_hour;
unsigned char temp_minute;
int temp_quantities[8];
char temp_pill_names[8][20];

char *temp_string;
char *send_ptr;
char *end_ptr;

char *array_ptr;
char *stop_array_ptr;

int ADC_value = 0;


#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void){
    switch(__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG)){
        case ADC12IV_ADC12IFG0: // ADC12MEM0 Interrupt
                ADC_value = ADC12MEM0; // Save MEM0
                __bic_SR_register_on_exit(LPM0_bits);
                break;
        default: break;
    }
}



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

void check_params(char *RX_data){


    int param = 0;
    
    temp_string = strstr(RX_data,"param");
    char hour_idx = 0;
    array_ptr = strchr(temp_string, ':') + 1;

    param = array_ptr[0] - '0';

    switch(param){
        case receiveAddReminderParam:
            receive_add_reminder();
            break;
        case receiveRemoveReminderParam:
            receive_remove_reminder();
            break;
        case receiveRefillParam:
            receive_refill();
            break;
        case receiveAddPillParam:
            receive_add_pill();
            break;
        case receiveRemovePillParam:
            receive_remove_pill();
            break;
    }

}

void receive_add_reminder(){
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

    if((strstr(RX_data, pillNames_field)) != NULL){ //pillNames is in the string
        temp_string = strstr(RX_data, pillNames_field);
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
    add_alarm(temp_hour, temp_minute, temp_quantities);
}

void receive_remove_reminder(){
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
    remove_alarm(temp_hour, temp_minute, temp_quantities);
}

void receive_refill(){
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
    refill(temp_quantities);
}

void receive_add_pill(){

    if((strstr(RX_data, "pillName")) != NULL){ //pillNames is in the string
        temp_string = strstr(RX_data, "pillName");
        char string_idx = 0;
        char char_idx = 0;
        array_ptr = strchr(temp_string, ':') + 1;
        stop_array_ptr = strchr(temp_string, ' ');

        if(stop_array_ptr == NULL){
            error = true;
        }

        while(array_ptr[0] != stop_array_ptr[0]){
                 temp_pill_names[0][char_idx++] = array_ptr[0];
                 array_ptr++;
        }
        temp_pill_names[0][char_idx] = '\0';
    }


    if((strstr(RX_data,"pillQuantity")) != NULL){ //pillQuantities is in the string
        temp_string = strstr(RX_data, "pillQuantity");
        char char_idx = 0;
        array_ptr = strchr(temp_string, ':') + 1;
        stop_array_ptr = strchr(temp_string, ' ');

        if(stop_array_ptr == NULL){
            error = true;
        }

        temp_quantities[0] = 0;
        //assume two digits
        while(array_ptr[0] != stop_array_ptr[0]){
             if(char_idx*16)
                temp_hour += array_ptr[0] - '0';
             else
                temp_hour += (array_ptr[0] - '0')*10; //sends hour in Hex BCD
             array_ptr++;
             char_idx++;
        }
    }
    add_pill(temp_pill_names[0], temp_quantities[0]);
}

void receive_remove_pill(){
    if((strstr(RX_data, "pillName")) != NULL){ //pillNames is in the string
        temp_string = strstr(RX_data, "pillName");
        char string_idx = 0;
        char char_idx = 0;
        array_ptr = strchr(temp_string, ':') + 1;
        stop_array_ptr = strchr(temp_string, ' ');

        if(stop_array_ptr == NULL){
            error = true;
        }

        while(array_ptr[0] != stop_array_ptr[0]){
                 temp_pill_names[0][char_idx++] = array_ptr[0];
                 array_ptr++;
        }
        temp_pill_names[0][char_idx] = '\0';
    }
    remove_pill(temp_pill_names[0]);
}

__int8_t getOldestAlarm(){
    return 0; //TODO: update to calculate oldest alarm from the lcd_control global vars
}

__int8_t getCurrentAlarm(){
    return 0; //TODO: update to use the current Alarm from the lcd_control global vars
}

// send UART build ptr

//param:1
char* build_analytics(){

    char currentAlarm = getCurrentAlarm();

    //"Hour:19 Minute:15 pillNames:[wept,Milton,did,op,one,bai,bee,boy] pillQuantities:[49,12,81,26,57,105,201,304]";

    strcpy(TXSendBuffer, "param:1 ");

    strcat(TXSendBuffer, hour_field);
    strcat(TXSendBuffer, ":");
    ltoa(schedule[currentAlarm].hour, temp_string, 16);
    if(schedule[currentAlarm].hour < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }
    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");


    strcat(TXSendBuffer, minute_field);
    strcat(TXSendBuffer, ":");
    ltoa(schedule[currentAlarm].minute, temp_string, 16);
    if(schedule[currentAlarm].minute < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }
    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, pillNames_field);
    strcat(TXSendBuffer, ":[");

    int i;
    for(i = 0; i< 8; i++){
        strcat(TXSendBuffer, schedule[currentAlarm].pill_names[i]);

        if(i < 7)
            strcat(TXSendBuffer, ",");
    }
    strcat(TXSendBuffer, "] ");


    strcat(TXSendBuffer, pillQuantities_field);
    strcat(TXSendBuffer, ":[");

    for(i = 0; i< 8; i++){
        strcat(TXSendBuffer, ltoa(schedule[currentAlarm].quantities[i], temp_string, 10));

        if(i < 7)
            strcat(TXSendBuffer, ",");
    }
    strcat(TXSendBuffer, "] ");

    strcat(TXSendBuffer, "Day:");
    ltoa(RTCDAY, temp_string, 16);

    if(RTCDAY < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }

    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, "Month:");
    ltoa(RTCMON, temp_string, 16);
    if(RTCMON < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }
    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, "Year:");
    strcat(TXSendBuffer, ltoa(RTCYEAR, temp_string, 16));
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, "DOW:");
    strcat(TXSendBuffer, ltoa(RTCDOW, temp_string, 16));
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, "TakenH:");
    ltoa(RTCHOUR, temp_string, 16);
    if(RTCHOUR < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }
    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, "TakenM:");
    ltoa(RTCMIN, temp_string, 16);
    if(RTCMIN < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }
    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, "Taken:");
    strcat(TXSendBuffer, ltoa(schedule[currentAlarm].taken, temp_string, 10));
    strcat(TXSendBuffer, " ");

    return TXSendBuffer;
}

//param:2
char* build_network_data(){

    strcpy(TXSendBuffer, "param:2 ");

    strcat(TXSendBuffer, "ssid");
    strcat(TXSendBuffer, ":");
    strcat(TXSendBuffer, SSID);
    strcat(TXSendBuffer, " ");


    strcat(TXSendBuffer, "password");
    strcat(TXSendBuffer, ":");
    strcat(TXSendBuffer, PASSWORD);
    strcat(TXSendBuffer, " ");

    return TXSendBuffer;
}

//param:3
char* build_pills_info(){

    strcpy(TXSendBuffer, "param:3 ");

    strcat(TXSendBuffer, pillNames_field);
    strcat(TXSendBuffer, ":[");

    int i;
    for(i = 0; i< 8; i++){
        strcat(TXSendBuffer, pill_names[i]);

        if(i < 7)
            strcat(TXSendBuffer, ",");
    }
    strcat(TXSendBuffer, "] ");


    strcat(TXSendBuffer, pillQuantities_field);
    strcat(TXSendBuffer, ":[");

    for(i = 0; i< 8; i++){
        strcat(TXSendBuffer, ltoa(quantities[i], temp_string, 10));

        if(i < 7)
            strcat(TXSendBuffer, ",");
    }
    strcat(TXSendBuffer, "] ");

    return TXSendBuffer;
}

//param:4
char* build_add_reminder_data(unsigned char index){

    strcpy(TXSendBuffer, "param:4 ");

    strcat(TXSendBuffer, "storeIndex:");
    strcat(TXSendBuffer, ltoa(index, temp_string, 10));
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, hour_field);
    strcat(TXSendBuffer, ":");
    ltoa(schedule[index].hour, temp_string, 16);
    if(schedule[index].hour < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }
    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, minute_field);
    strcat(TXSendBuffer, ":");
    ltoa(schedule[index].minute, temp_string, 16);
    if(schedule[index].minute < 0x10){
        temp_string[1] = temp_string[0];
        temp_string[0] = '0';
        temp_string[2] = '\0';
    }
    strcat(TXSendBuffer, temp_string);
    strcat(TXSendBuffer, " ");

    strcat(TXSendBuffer, pillQuantities_field);
    strcat(TXSendBuffer, ":[");

    int i;
    for(i = 0; i< 8; i++){
        strcat(TXSendBuffer, ltoa(quantities[i], temp_string, 10));

        if(i < 7)
            strcat(TXSendBuffer, ",");
    }
    strcat(TXSendBuffer, "] ");

    return TXSendBuffer;
}

//param:5
char* build_remove_reminder_data(unsigned char index){
    strcpy(TXSendBuffer, "param:5 ");

    strcat(TXSendBuffer, "storeIndex:");
    strcat(TXSendBuffer, ltoa(index, temp_string, 10));
    strcat(TXSendBuffer, " ");

    return TXSendBuffer;
}

//param:6
char* build_refill_pills(){

    strcpy(TXSendBuffer, "param:6 ");


    strcat(TXSendBuffer, pillQuantities_field);
    strcat(TXSendBuffer, ":[");
    int i;
    for(i = 0; i< 8; i++){
        strcat(TXSendBuffer, ltoa(quantities[i], temp_string, 10));

        if(i < 7)
            strcat(TXSendBuffer, ",");
    }
    strcat(TXSendBuffer, "] ");

    return TXSendBuffer;
}

//param:7
char* build_add_pill_data(char index){
    strcpy(TXSendBuffer, "param:7 ");

    strcat(TXSendBuffer, "storeIndex:");
    strcat(TXSendBuffer, ltoa(index, temp_string, 10));

    strcat(TXSendBuffer, "pillName:");
    strcat(TXSendBuffer, pill_names[index]);
    strcat(TXSendBuffer, " ");

    
    strcat(TXSendBuffer, "pillQuantity:");
    strcat(TXSendBuffer, ltoa(quantities[index], temp_string, 10));
    strcat(TXSendBuffer, " ");

    return TXSendBuffer;
}

void send_uart(char param, char index){ //this function sends the data from MSP430 temporary variables to the UART port
    //TODO: add send_uart to timer_isr while checking flags in order to set param accordingly

    switch(param){
        case sendAnalyticsParam:
            send_ptr = build_analytics();
            break;
        case sendNetworkParam:
            send_ptr = build_network_data();
            break;
        case sendPillInfoParam:
            send_ptr = build_pills_info();
            break;
        case sendAddReminderParam:
            send_ptr = build_add_reminder_data(index);
            break;
        case sendRemoveReminderParam:
            send_ptr = build_remove_reminder_data(index);
            break;
        case sendRefillParam:
            send_ptr = build_refill_pills();
            break;
        case sendAddPillParam:
            send_ptr = build_add_pill_data(index);
            break;
        case sendRemovePillParam:
            send_ptr = build_remove_reminder_data(index);
            break;
        
        default: send_ptr = "";
    }

    int length = strlen(send_ptr);

    while(length){

        while(!(UCA0IFG & UCTXIFG)); // Wait for TX buffer to be ready for new data


            UCA0TXBUF = *send_ptr; // Push data to TX buffer

            // Update variables
            length--;
            send_ptr++;
    }

    // Wait until the last byte is completely sent
    while(UCA0STATW & UCBUSY);
}
