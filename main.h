/*
 * main.h
 *
 *  Created on: Apr 21, 2021
 *      Author: Milton
 */

#ifndef MAIN_H_
#define MAIN_H_
#pragma once

#include <msp430fr6989.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "lcd_control.h"

struct alarm
{
    unsigned char hour;
    unsigned char minute;
    int quantities[8];
    char pill_names[8][15];
};

//int pill_quantities[8];
//char pill_names[8][15];
struct alarm schedule[8];


#endif /* MAIN_H_ */
