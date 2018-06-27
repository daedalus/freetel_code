/*
 * Copyright (C) 1993-2018 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#pragma once

#include <stdint.h>

/* Mode 700D uses 4 Vocoder Frames */

#define ADC_BUF_SZ   (320 * 4)

/* divisors for various sample rates */

#define ADC_FS_8KHZ 10500
#define ADC_FS_16KHZ 5250
#define ADC_FS_48KHZ 1750
#define ADC_FS_96KHZ 875

void adc_open(int, int);

int adc1_read(int16_t [], int); /* ADC1 Pin PA1 */
int adc2_read(int16_t [], int); /* ADC2 Pin PA2 */

int adc1_samps(void);
int adc2_samps(void);

