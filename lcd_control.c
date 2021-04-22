#include "main.h"

void display_init()
{
    //    LCD_DATA_DIR |= 0xFF;
    //    LCD_DATA_OUT &= 0x00;
    //    LCD_CONTROL_DIR |= RS + EN;  // Set control signal pins as output
    //    LCD_CONTROL_OUT &= ~RS & ~EN;
    //
    //    __delay_cycles((unsigned long) 100 * MS_CYCLES);
    //    LCD_CONTROL_OUT &= ~RS;
    //    send_nibble(0x30);
    //    __delay_cycles((unsigned long) 5 * MS_CYCLES);
    //    send_nibble(0x30);
    //    __delay_cycles((unsigned long) 500 * US_CYCLES);
    //    send_command(0x38);
    //    __delay_cycles((unsigned long) 500 * US_CYCLES);
    //    display_off();
    //    __delay_cycles((unsigned long) 500 * US_CYCLES);
    //    clear_display();
    //    __delay_cycles((unsigned long) 500 * US_CYCLES);
    //    send_command(0x06);
    //    __delay_cycles((unsigned long) 500 * US_CYCLES);
    //    send_command(0x0F);
    //    __delay_cycles((unsigned long) 500 * US_CYCLES);

    // P1DIR |= 0xFF; //set output direction
    P9DIR |= 0xFF; // set output direction
    //P1OUT &= 0x00; // reset port 2
    P9OUT &= 0x00; // reset port 1
                   //        send_command(0x38); // function set
                   //        send_command(0x0F); // display control
                   //        clear_display();       // clear display
                   //        send_command(0x06); // entry mode set
                   //        send_command(0x80); //set cursor in address 40

    send_nibble(0x33);
    send_nibble(0x32);
    send_nibble(0x28); // 4 bit mode
    send_nibble(0x0E); // clear the screen
    send_nibble(0x01); // display on cursor on
    send_nibble(0x06); // increment cursor
    send_nibble(0x80); // row 1 column 1
}

void enable_cycle()
{
    // Carries out one enable cycle for the LCD
    // Minimum pulse width is 450ns
    LCD_CONTROL_OUT &= ~EN;
    // E_L;
    __delay_cycles(US_CYCLES);
    LCD_CONTROL_OUT |= EN;
    //E_H;
    __delay_cycles(US_CYCLES);
    LCD_CONTROL_OUT &= ~EN;
    //E_L;
    __delay_cycles((unsigned long)100 * US_CYCLES);
}

void send_command(unsigned char command)
{
    LCD_CONTROL_OUT &= ~RS;
    // RS_L;
    send_nibble(command);
}

void send_character(unsigned char character)
{
    LCD_CONTROL_OUT |= RS;
    //RS_H;
    send_nibble(character);
    //RS_L;
    LCD_CONTROL_OUT &= ~RS;
}

void send_nibble(unsigned char data)
{
    __delay_cycles((unsigned long)500 * US_CYCLES);
    LCD_DATA_OUT = (LCD_DATA_OUT & 0xF0) | ((data >> 4) & 0x0F);
    enable_cycle();
    LCD_DATA_OUT = (LCD_DATA_OUT & 0xF0) | (data & 0x0F);
    enable_cycle();
}

void send_byte(unsigned char data)
{
    __delay_cycles((unsigned long)500 * US_CYCLES);
    LCD_DATA_OUT = data;
    enable_cycle();
}

void send_string(char str[], unsigned char row)
{
    int i;
    for (i = 0; str[i] != 0x00; i++)
    {
        //set_cursor(row, i);
        send_character(str[i]);
    }
}

void clear_display()
{
    LCD_CONTROL_OUT &= ~RS;
    //RS_L;
    send_nibble(0x01); // Clear display
    __delay_cycles((unsigned long)2 * MS_CYCLES);
}

void display_off()
{
    send_command(0x08);
}

void set_cursor(unsigned char row, unsigned char column)
{
    unsigned char command = row ? (0x80 | (column | 0x40)) : (0x80 | (column + 0x00));
    send_command(command);
}
