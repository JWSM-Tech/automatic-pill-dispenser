/*
 * main.h
 *
 *  Created on: Apr 21, 2021
 *      Author: milpa
 */

#ifndef MAIN_H_
#define MAIN_H_
#pragma once

#include <msp430fr6989.h>

struct alarm
{
    unsigned char hour;
    unsigned char minute;
    int quantities[8];
    char pill_names[8][20];
};
struct alarm schedule;

#endif /* MAIN_H_ */
