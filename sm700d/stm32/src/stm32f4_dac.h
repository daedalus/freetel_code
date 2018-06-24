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

#define DAC_BUF_SZ   640

/* divisors for various sample rates */

#define DAC_FS_8KHZ 10500
#define DAC_FS_16KHZ 5250
#define DAC_FS_48KHZ 1750
#define DAC_FS_96KHZ 875

void dac_open(int, int);

int dac1_write(int16_t [], int); /* DAC1 pin PA4 */
int dac2_write(int16_t [], int); /* DAC2 pin PA5 */

int dac1_free(void);
int dac2_free(void);

