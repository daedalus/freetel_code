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

void ofdm_leds_switches_init(void);

#define	LED_ON	1	/* Turn LED on */
#define LED_OFF	0	/* Turn LED off */
#define LED_INV	-1	/* Invert LED state */

#define PTT_ON  1
#define PTT_OFF 0

void led_pwr(int);
void led_ptt(int);
void led_sync(int);
void led_err(int);

void rig_ptt(int);

void debug_0(int);
void debug_1(int);
void debug_2(int);
void debug_3(int);

int switch_ptt(void);
int switch_select(void);
int switch_back(void);

int ext_ptt(void);

#define DEBOUNCE_DELAY 50 /* Delay to wait while switch bounces */

#define SW_STEADY   0   /* Switch is in steady-state */
#define SW_DEBOUNCE 1   /* Switch is being debounced */

/* Switch debounce and logic handling */
struct switch_t
{
    /* Debounce/hold timer */
    uint32_t    timer;

    /* Current/debounced observed switch state */
    uint8_t     sw;

    /* Raw observed switch state (during debounce) */
    uint8_t     raw;

    /* Last steady-state switch state */
    uint8_t     last;

    /* Debouncer state */
    uint8_t     state;
};

/* Update the state of a switch */
void switch_update(struct switch_t *const, uint8_t);

/* Acknowledge the current state of the switch */
void switch_ack(struct switch_t *const);

/* Return how long the switch has been pressed in ticks. */
uint32_t switch_pressed(const struct switch_t *const);

/* Return non-zero if the switch has been released. */
int switch_released(const struct switch_t *const);

/* Count the tick timers on the switches. */
void switch_tick(struct switch_t *const);

void ColorfulRingOfDeath(int);

