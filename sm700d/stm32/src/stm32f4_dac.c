/*
 * Copyright (C) 1993-2018 David Rowe
 *
 * All rights reserved
 *
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 *
 * DAC driver module for STM32F4. DAC1 is connected to pin PA4, DAC2
 * is connected to pin PA5.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stm32f4xx.h>

#include "codec2_fifo.h"
#include "stm32f4_dac.h"
#include "ofdm_leds_switches.h"

/* write to these registers for 12 bit left aligned data, as per data sheet
   make sure 4 least sig bits set to 0 */

#define DAC_DHR12R1_ADDRESS    0x40007408
#define DAC_DHR12R2_ADDRESS    0x40007414

#define DAC_MAX      4096            /* maximum amplitude */

/* y=mx+c mapping of samples16 bit shorts to DAC samples.  Table: 74
   of data sheet indicates With DAC buffer on, DAC range is limited to
   0x0E0 to 0xF1C at VREF+ = 3.6 V, we have Vref=3.3V which is close.
 */

#define Slope ((3868.0f - 224.0f) / 65536.0f)
#define Constant 2047.0f

static struct FIFO *dac1_fifo;
static struct FIFO *dac2_fifo;

static uint16_t dac1_buf[DAC_BUF_SZ];
static uint16_t dac2_buf[DAC_BUF_SZ];

static void tim6_config(int fs_divisor);
static void dac1_config(void);
static void dac2_config(void);

int dac_underflow;

void dac_open(int fs_divisor, int fifo_size) {
    memset(dac1_buf, 32768, sizeof(uint16_t) * DAC_BUF_SZ);
    memset(dac2_buf, 32768, sizeof(uint16_t) * DAC_BUF_SZ);

    /* Turn on the clocks we need -----------------------------------------------*/

    /* DMA1 clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    /* GPIOA clock enable (to be used with DAC) */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* DAC Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    /* GPIO Pin configuration DAC1->PA.4, DAC2->PA.5 configuration --------------*/

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Timer and DAC 1 & 2 Configuration ----------------------------------------*/

    tim6_config(fs_divisor);

    /* Create fifos */

    dac1_fifo = fifo_create(fifo_size);
    dac2_fifo = fifo_create(fifo_size);

    dac1_config();
    dac2_config();
}

/* Call these puppies to send samples to the DACs.  For your
   convenience they accept signed 16 bit samples. */

int dac1_write(int16_t buf[], int n) {
    return fifo_write(dac1_fifo, buf, n);
}

int dac2_write(int16_t buf[], int n) {
    return fifo_write(dac2_fifo, buf, n);
}

int dac1_free() {
    return fifo_free(dac1_fifo);
}

int dac2_free() {
    return fifo_free(dac2_fifo);
}

static void tim6_config(int fs_divisor) {
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* --------------------------------------------------------

  TIM6 input clock (TIM6CLK) is set to 2 * APB1 clock (PCLK1), since
  APB1 prescaler is different from 1 (see system_stm32f4xx.c and Fig
  13 clock tree figure in DM0031020.pdf).

     Sample rate Fs = 2*PCLK1/TIM_ClockDivision
                    = (HCLK/2)/TIM_ClockDivision

  ----------------------------------------------------------- */

  /* Time base configuration */

  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = fs_divisor - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */

  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

  /* TIM6 enable counter */

  TIM_Cmd(TIM6, ENABLE);
}

static void dac1_config() {
  DAC_InitTypeDef  DAC_InitStructure;
  DMA_InitTypeDef  DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* DAC channel 1 Configuration */

  /*
     This line fixed a bug that cost me 5 days, bad wave amplitude
     value, and some STM32F4 periph library bugs caused triangle wave
     generation to be enable resulting in a low level tone on the
     hardware, that we thought was caused by analog issues like layout
     or power supply biasing
  */
  DAC_StructInit(&DAC_InitStructure);

  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /* DMA1_Stream5 channel7 configuration **************************************/
  /* Table 35 page 219 of the monster data sheet */

  DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R1_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)dac1_buf;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = DAC_BUF_SZ;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure);

  /* Enable DMA Half & Complete interrupts */

  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_HT, ENABLE);

  /* Enable the DMA Stream IRQ Channel */

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA1_Stream5 */

  DMA_Cmd(DMA1_Stream5, ENABLE);

  /* Enable DAC Channel 1 */

  DAC_Cmd(DAC_Channel_1, ENABLE);

  /* Enable DMA for DAC Channel 1 */

  DAC_DMACmd(DAC_Channel_1, ENABLE);
}

static void dac2_config() {
  DAC_InitTypeDef  DAC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* DAC channel 2 Configuration (see notes in dac1_config() above) */

  DAC_StructInit(&DAC_InitStructure);
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* DMA1_Stream6 channel7 configuration **************************************/

  DMA_DeInit(DMA1_Stream6);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R2_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)dac2_buf;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = DAC_BUF_SZ;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  /* Enable DMA Half & Complete interrupts */

  DMA_ITConfig(DMA1_Stream6, DMA_IT_TC | DMA_IT_HT, ENABLE);

  /* Enable the DMA Stream IRQ Channel */

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA1_Stream6 */

  DMA_Cmd(DMA1_Stream6, ENABLE);

  /* Enable DAC Channel 2 */

  DAC_Cmd(DAC_Channel_2, ENABLE);

  /* Enable DMA for DAC Channel 2 */

  DAC_DMACmd(DAC_Channel_2, ENABLE);

}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f40xx.s/startup_stm32f427x.s).                         */
/******************************************************************************/

/*
  This function handles DMA1 Stream 5 interrupt request for DAC1.
*/

void DMA1_Stream5_IRQHandler() {
    int i, j, sam;
    int16_t signed_buf[DAC_BUF_SZ / 2];

    debug_1(LED_ON);

    /* Transfer half empty interrupt - refill first half */

    if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5) != RESET) {
        /* fill first half from fifo */

        if (fifo_read(dac1_fifo, signed_buf, DAC_BUF_SZ/2) == -1) {
            memset(signed_buf, 0, sizeof(int16_t) * (DAC_BUF_SZ / 2));
            dac_underflow++;
        }

        /* convert to unsigned */

        for(i=0; i<DAC_BUF_SZ/2; i++) {
            sam = (int)(Slope * (float)signed_buf[i] + Constant);
            dac1_buf[i] = (uint16_t) sam;
        }

        /* Clear DMA Stream Transfer Complete interrupt pending bit */

        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
    }

    /* Transfer complete interrupt - refill 2nd half */

    if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5) != RESET) {
        /* fill second half from fifo */

        if (fifo_read(dac1_fifo, signed_buf, DAC_BUF_SZ/2) == -1) {
            memset(signed_buf, 0, sizeof(int16_t)*DAC_BUF_SZ/2);
            dac_underflow++;
        }

        /* convert to unsigned */

        for(i=0, j=DAC_BUF_SZ/2; i<DAC_BUF_SZ/2; i++,j++) {
            sam = (int)(Slope * (float)signed_buf[i] + Constant);
            dac1_buf[j] = (uint16_t) sam;
        }

        /* Clear DMA Stream Transfer Complete interrupt pending bit */

        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
    }

    debug_1(LED_OFF);
}

/*
  This function handles DMA1 Stream 6 interrupt request for DAC2.
*/

void DMA1_Stream6_IRQHandler() {
    int i, j, sam;
    int16_t signed_buf[DAC_BUF_SZ / 2];

    debug_2(LED_ON);

    /* Transfer half empty interrupt - refill first half */

    if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_HTIF6) != RESET) {
        /* fill first half from fifo */

        if (fifo_read(dac2_fifo, signed_buf, DAC_BUF_SZ/2) == -1) {
            memset(signed_buf, 0, sizeof(int16_t) * (DAC_BUF_SZ / 2));
            dac_underflow++;
        }

        /* convert to unsigned */

        for(i=0; i<DAC_BUF_SZ/2; i++) {
            sam = (int)(Slope * (float)signed_buf[i] + Constant);
            dac2_buf[i] = (uint16_t)sam;
        }

        /* Clear DMA Stream Transfer Complete interrupt pending bit */

        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_HTIF6);
    }

    /* Transfer complete interrupt - refill 2nd half */

    if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET) {
        /* fill second half from fifo */

        if (fifo_read(dac2_fifo, signed_buf, DAC_BUF_SZ/2) == -1) {
            memset(signed_buf, 0, sizeof(int16_t)*DAC_BUF_SZ/2);
            dac_underflow++;
        }

        /* convert to unsigned  */

        for(i=0, j=DAC_BUF_SZ/2; i<DAC_BUF_SZ/2; i++,j++) {
            sam = (int)(Slope * (float)signed_buf[i] + Constant);
            dac2_buf[j] = (uint16_t)sam;
        }

        /* Clear DMA Stream Transfer Complete interrupt pending bit */

        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
    }

    debug_2(LED_OFF);
}

