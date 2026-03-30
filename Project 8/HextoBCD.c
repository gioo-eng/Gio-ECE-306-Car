/*
 * HextoBCD.c
 *
 *  Created on: Mar 4, 2026
 *      Author: yanny
 */
#include "msp430.h"
#include "functions.h"
#include "LCD.h"
#include "macros.h"
#include <string.h>


char adc_char[5];


void HEXtoBCD(int hex_value) {
    int value;
   unsigned int i = 0;


    // 1. Clear/Initialize the array with ASCII '0' (0x30)
    for(i=0; i < 4; i++) {
        adc_char[i] = '0';
    }

    // 2. Thousands
    value = 0;
    while (hex_value >= 1000) { // Use >= to catch exactly 1000
        hex_value -= 1000;
        value++;
    }
    adc_char[0] = 0x30 + value; // Store the final count after the loop

    // 3. Hundreds
    value = 0;
    while (hex_value >= 100) {
        hex_value -= 100;
        value++;
    }
    adc_char[1] = 0x30 + value;

    // 4. Tens
    value = 0;
    while (hex_value >= 10) {
        hex_value -= 10;
        value++;
    }
    adc_char[2] = 0x30 + value;

    // 5. Ones
    adc_char[3] = 0x30 + hex_value;
    adc_char[4] = '\0';
}

