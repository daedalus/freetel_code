/*
 * Copyright (C) 1993-2018 David Rowe
 *
 * All rights reserved
 *
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

#include "freedv_api.h"
#include "ofdm_internal.h"
#include "codec2_ofdm.h"

#include "stm32f4_adc.h"
#include "stm32f4_dac.h"
#include "freedv_api.h"
#include "ofdm_leds_switches.h"

#define STATE_RX        0x00    /* Receive state: normal operation */
#define STATE_TX        0x01    /* Transmit state: normal operation */

#define MAX_MODES  2
#define ANALOG     0
#define DIGITAL    1

struct switch_t sw_select;  /* Switch driver for SELECT button */
struct switch_t sw_back;    /* Switch driver for BACK button */
struct switch_t sw_ptt;     /* Switch driver for PTT buttons */

static uint32_t menuTicker = 0;

static bool mode_changed = false;

static int16_t *adc8k;
static int16_t *dac8k;

static int spk_nsamples;
static int n_samples;

/*
 * SysTick Interrupt Handler
 */

void SysTick_Handler() {
    switch_tick(&sw_select);
    switch_tick(&sw_back);
    switch_tick(&sw_ptt);

    if (menuTicker > 0) {
        menuTicker--;
    }
}

/*------------------------------------------- main -------------------------------*/

int main() {
    struct freedv *f;
    int16_t *play_ptr;
    int i, op_mode, len, core_state;

    /* init all the drivers for various peripherals */

    SysTick_Config(SystemCoreClock/1000); /* CMSIS function 1 kHz SysTick */
    ofdm_leds_switches_init();

    /* Enable CRC clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);

    /* Set up ADCs/DACs */
    dac_open(DAC_FS_8KHZ, DAC_BUF_SZ * 4);
    adc_open(ADC_FS_8KHZ, ADC_BUF_SZ * 4);

    if ((f = freedv_open(FREEDV_MODE_700D)) == NULL) {
        ColorfulRingOfDeath(5);		/* you can read this value under GDB */
    }

    /* initialize state */

    op_mode = ANALOG;
    core_state = STATE_RX;

    spk_nsamples = 0;       /* Outgoing sample counter */
    n_samples = 320;        /* 320 Analog setting */

    adc8k = calloc(1, n_samples * sizeof (int16_t));
    dac8k = calloc(1, n_samples * sizeof (int16_t));

    led_pwr(LED_ON);
    led_sync(LED_OFF);
    led_err(LED_OFF);
    led_ptt(LED_OFF);
    rig_ptt(PTT_OFF);

    /* Clear out switch states */

    memset(&sw_select, 0, sizeof(sw_select));
    memset(&sw_back, 0, sizeof(sw_back));
    memset(&sw_ptt, 0, sizeof(sw_ptt));

    while (1) {
        /* Read switch states */

        switch_update(&sw_select, switch_select());
        switch_update(&sw_back, switch_back());
        switch_update(&sw_ptt, (switch_ptt() || ext_ptt()));

        /* State machine updates */

        switch (core_state) {
            case STATE_RX:
                {
                    mode_changed = false;

                    if (!menuTicker) {
                        if (switch_pressed(&sw_ptt)) {

                            /* Enter transmit state */

                            core_state = STATE_TX;
                        } else if (switch_released(&sw_select)) {
                            op_mode = (op_mode + 1) % MAX_MODES;
                            mode_changed = true;
                        } else if (switch_released(&sw_back)) {
                            op_mode = (op_mode - 1) % MAX_MODES;
                            mode_changed = true;
                        }

                        if (mode_changed == true) {
                            free(adc8k);
                            free(dac8k);

                            if (op_mode == ANALOG) {
                                n_samples = 320;
                            } else if (op_mode == DIGITAL) {
                                n_samples = freedv_get_n_speech_samples(f) * 4;    /* OFDM has 4 Vocoder Frames */
                            }

                            adc8k = calloc(1, n_samples * sizeof (int16_t));
                            dac8k = calloc(1, n_samples * sizeof (int16_t));
                        }
                    }
                }
                break;
            case STATE_TX:
                if (!switch_pressed(&sw_ptt)) {

                    /* PTT released, leave transmit mode */

                    core_state = STATE_RX;
                }
                break;
            default:
                break;
        }

        /* Acknowledge switch events */

        switch_ack(&sw_select);
        switch_ack(&sw_back);
        switch_ack(&sw_ptt);

        switch (core_state) {
            case STATE_TX:

                led_ptt(LED_ON);
                led_sync(LED_OFF);
                led_err(LED_OFF);

                rig_ptt(PTT_ON);

                if (op_mode == ANALOG) {
                    /* ADC2 is the microphone */
                    /* DAC1 is the modulator signal we send to radio tx */

                    debug_3(LED_ON);

                    if (adc2_read(adc8k, n_samples) == 0) {

                        /* clipping indicator */
                        led_err(LED_OFF);

                        for (i = 0; i < n_samples; i++) {
                            if (abs(adc8k[i]) > 28000) {
                                led_err(LED_ON);
                            }
                        }

                        for (i = 0; i < n_samples; i++)
                            dac8k[i] = adc8k[i];

                        dac1_write(dac8k, n_samples);
                    }

                    debug_3(LED_OFF);

                } else if (op_mode == DIGITAL) {
                    /* ADC2 is the microphone */
                    /* DAC1 is the modulator signal we send to radio tx */

                    debug_3(LED_ON);

                    if (adc2_read(adc8k, n_samples) == 0) {

                        /* clipping indicator */
                        led_err(LED_OFF);

                        for (i = 0; i < n_samples; i++) {
                            if (abs(adc8k[i]) > 28000) {
                                led_err(LED_ON);
                            }
                        }

                        freedv_tx(f, dac8k, adc8k);

                        dac1_write(dac8k, n_samples);
                    }

                    debug_3(LED_OFF);
                }
                break;

            case STATE_RX:

                rig_ptt(PTT_OFF);
                led_ptt(LED_OFF);

                /* ADC1 is the demod in signal from the radio rx */
                /* DAC2 is the modem speaker */

                if (op_mode == ANALOG) {
                    if (adc1_read(adc8k, n_samples) == 0) {

                        for(i = 0; i < n_samples; i++)
                            dac8k[i] = adc8k[i];

                        spk_nsamples = n_samples;

                        led_sync(LED_OFF);
                        led_err(LED_OFF);
                   }
                } else if (op_mode == DIGITAL) {
                    int nin = freedv_nin(f);
                    int nout = nin;

                    freedv_set_total_bit_errors(f, 0);

                    if (adc1_read(adc8k, nin) == 0) {
                        debug_3(LED_ON);

                        nout = freedv_rx(f, dac8k, adc8k);

                        spk_nsamples = nout;

                        led_sync(freedv_get_sync(f));
                        led_err(freedv_get_total_bit_errors(f));

                        debug_3(LED_OFF);
                    }
                }
        }

        /* Write audio to speaker output */

        if (spk_nsamples) {

            play_ptr = dac8k;

            while (spk_nsamples != 0) {
                /* Get the number of samples to be played this time around */

                len = dac2_free();

                if (len) {
                    if (spk_nsamples < len)
                        len = spk_nsamples;

                    /* Play the audio */

                    dac2_write(play_ptr, len);
                    spk_nsamples -= len;
                    play_ptr += len;
                }
            }

            /* Clear out buffer */

            memset(dac8k, 0, n_samples);
        }
    }
}

