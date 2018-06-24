/*
 * Copyright (C) 1993-2017 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 *
 * Functions for controlling LEDs and reading switches on the hardware.
 */

#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>

#include "ofdm_leds_switches.h"

#define _CPTT          GPIO_Pin_10
#define LED_PWR        GPIO_Pin_12
#define LED_PTT        GPIO_Pin_13
#define LED_SYNC       GPIO_Pin_14
#define LED_ERR        GPIO_Pin_15
#define SWITCH_PTT     GPIO_Pin_7
#define SWITCH_SELECT  GPIO_Pin_0
#define SWITCH_BACK    GPIO_Pin_1
#define EXT_PTT        GPIO_Pin_8

void ofdm_leds_switches_init() {
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* output pins */

    GPIO_InitStruct.GPIO_Pin = LED_PWR | LED_PTT | LED_SYNC | LED_ERR | _CPTT;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* input pins */

    GPIO_InitStruct.GPIO_Pin = SWITCH_PTT | SWITCH_SELECT | SWITCH_BACK;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; /* we have our own external pull ups */
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = EXT_PTT;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; /* use internal pull up */
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* PE0-3 used to indicate activity */

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void debug_0(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOE, GPIO_Pin_0);
    else
        GPIO_ResetBits(GPIOE, GPIO_Pin_0);
}

void debug_1(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOE, GPIO_Pin_1);
    else
        GPIO_ResetBits(GPIOE, GPIO_Pin_1);
}

void debug_2(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOE, GPIO_Pin_2);
    else
        GPIO_ResetBits(GPIOE, GPIO_Pin_2);
}

void debug_3(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOE, GPIO_Pin_3);
    else
        GPIO_ResetBits(GPIOE, GPIO_Pin_3);
}

void led_pwr(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOD, GPIO_Pin_12);
    else if (state == LED_INV)
        GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
    else /* LED_OFF */
        GPIO_ResetBits(GPIOD, GPIO_Pin_12);
}

void led_ptt(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOD, GPIO_Pin_13);
    else if (state == LED_INV)
        GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
    else /* LED_OFF */
        GPIO_ResetBits(GPIOD, GPIO_Pin_13);
}

void led_sync(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOD, GPIO_Pin_14);
    else if (state == LED_INV)
        GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
    else /* LED_OFF */
        GPIO_ResetBits(GPIOD, GPIO_Pin_14);
}

void led_err(int state) {
    if (state == LED_ON)
        GPIO_SetBits(GPIOD, GPIO_Pin_15);
    else if (state == LED_INV)
        GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
    else /* LED_OFF */
        GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}

void rig_ptt(int state) {
    if (state == PTT_ON)			/* Rig PTT Enable (negative logic - flipped for user) */
        GPIO_ResetBits(GPIOD, GPIO_Pin_10);
    else if (state == PTT_OFF)			/* Rig PTT Disable */
        GPIO_SetBits(GPIOD, GPIO_Pin_10);
}

int switch_ptt() {
   return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7) ? 1 : 0;
}

int switch_select() {	/* negative logic - flipped for user */
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0) ? 0 : 1;
}

int switch_back() {	/* negative logic - flipped for user */
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1) ? 0 : 1;
}

int ext_ptt() {		/* negative logic - flipped for user */
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_8) ? 0 : 1;
}

/*
  FUNCTION: ColorfulRingOfDeath()
  AUTHOR..: xenovacivus

  Colourful ring of death, blink LEDs like crazy forever if something
  really nasty happens.  Adapted from USB Virtual COM Port (VCP)
  module adapted from code I found here:

    https://github.com/xenovacivus/STM32DiscoveryVCP

  Call this to indicate a failure.  Blinks the STM32F4 discovery LEDs
  in sequence.  At 168Mhz, the blinking will be very fast - about 5
  Hz.  Keep that in mind when debugging, knowing the clock speed
  might help with debugging.
 */

static int mycode; /* examine this with debugger if it dies */

void ColorfulRingOfDeath(int code) {
    mycode = code;
    uint32_t count;
    uint16_t ring = 1;
    
    while (1) {
        count = 0;

        while (count++ < 5000000)
            ;

	GPIO_ResetBits(GPIOD, (ring << 12));
        ring = (ring << 1);

        if (ring >= (1 << 4)) {
            ring = 1;
        }

	GPIO_SetBits(GPIOD, (ring << 12));
    }
}

void HardFault_Handler() {
    ColorfulRingOfDeath(1);
}

void MemManage_Handler() {
    ColorfulRingOfDeath(2);
}

void BusFault_Handler() {
    ColorfulRingOfDeath(3);
}

void UsageFault_Handler() {
    ColorfulRingOfDeath(4);
}

void switch_tick(struct switch_t *const sw) {
    if (sw->sw != sw->raw) {
        /* State transition, reset timer */
        if (sw->state == SW_STEADY)
            sw->last = sw->sw;

        sw->state = SW_DEBOUNCE;
        sw->timer = DEBOUNCE_DELAY;
        sw->sw = sw->raw;
    } else if (sw->state == SW_DEBOUNCE) {
        if (sw->timer > 0) {
            /* Steady so far, keep waiting */
            sw->timer--;
        } else {
            /* Steady state reached */
            sw->state = SW_STEADY;
        }
    } else if (sw->sw) {
        /* Hold state.  Yes this will wrap, but who cares? */
        sw->timer++;
    }
}

void switch_update(struct switch_t *const sw, uint8_t state) {
    sw->raw = state;

    if (sw->raw == sw->sw)
        return;

    if (sw->state == SW_STEADY)
        sw->last = sw->sw;

    sw->timer = DEBOUNCE_DELAY;
    sw->sw = sw->raw;
    sw->state = SW_DEBOUNCE;
}

uint32_t switch_pressed(const struct switch_t *const sw) {
    if ((sw->state == SW_STEADY) && sw->sw)
        return sw->timer;

    return 0;
}

int switch_released(const struct switch_t *const sw) {
    if (sw->state != SW_STEADY)
        return 0;

    if (!sw->last)
        return 0;

    if (sw->sw)
        return 0;

    return 1;
}

void switch_ack(struct switch_t *const sw) {
    if (sw->state == SW_STEADY)
        sw->last = sw->sw;
}

