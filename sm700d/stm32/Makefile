# Makefile for OFDM 700D Mode

CROSS_COMPILE ?= arm-none-eabi-
CC=$(BINPATH)$(CROSS_COMPILE)gcc
OBJCOPY=$(BINPATH)$(CROSS_COMPILE)objcopy
SIZE=$(BINPATH)$(CROSS_COMPILE)size
SUDO ?= sudo

# ---------------------------------------------------------------------------

CFLAGS  = -std=c99 -O3 -g -Wall -Tstm32_flash.ld -DSTM32F40_41xxx -DCORTEX_M4
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -fdata-sections -ffunction-sections -Xlinker --gc-sections
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard -D__FPU_PRESENT=1 -D__FPU_USED=1
CFLAGS += -DSTM700D

# ---------------------------------------------------------------------------

# Definitions for the STM32F4 Standard Peripheral Library

PERIPHLIBNAME	= STM32F4xx_DSP_StdPeriph_Lib
PERIPHLIBDIR	= $(PERIPHLIBNAME)
CMSIS		= $(PERIPHLIBDIR)/Libraries/CMSIS
STM32F4LIB	= $(PERIPHLIBDIR)/Libraries/STM32F4xx_StdPeriph_Driver
DSPLIB          = $(PERIPHLIBDIR)/Libraries/CMSIS/DSP_Lib

CFLAGS		+= -DUSE_STDPERIPH_DRIVER -I$(CMSIS)/Include -I$(CMSIS)/Device/ST/STM32F4xx/Include
CFLAGS		+= -I$(STM32F4LIB)/inc -DARM_MATH_CM4 -specs=nosys.specs

STM32F4LIB_SRCS=\
$(STM32F4LIB)/src/misc.c \
$(STM32F4LIB)/src/stm32f4xx_flash.c\
$(STM32F4LIB)/src/stm32f4xx_rcc.c\
$(STM32F4LIB)/src/stm32f4xx_gpio.c\
$(STM32F4LIB)/src/stm32f4xx_tim.c\
$(STM32F4LIB)/src/stm32f4xx_dma.c\
$(STM32F4LIB)/src/stm32f4xx_adc.c\
$(STM32F4LIB)/src/stm32f4xx_dac.c\
$(STM32F4LIB)/src/stm32f4xx_crc.c

STM32F4LIB_OBJS = $(STM32F4LIB_SRCS:.c=.o)

CMSIS_SRCS=\
$(CMSIS)/DSP_Lib/Source/BasicMathFunctions/arm_scale_f32.c\
$(CMSIS)/DSP_Lib/Source/CommonTables/arm_common_tables.c\
$(CMSIS)/DSP_Lib/Source/CommonTables/arm_const_structs.c\
$(CMSIS)/DSP_Lib/Source/FastMathFunctions/arm_cos_f32.c\
$(CMSIS)/DSP_Lib/Source/FastMathFunctions/arm_sin_f32.c\
$(CMSIS)/DSP_Lib/Source/TransformFunctions/arm_bitreversal.c\
$(CMSIS)/DSP_Lib/Source/TransformFunctions/arm_cfft_radix8_f32.c\
$(CMSIS)/DSP_Lib/Source/TransformFunctions/arm_cfft_f32.c\
$(CMSIS)/DSP_Lib/Source/TransformFunctions/arm_rfft_f32.c\
$(CMSIS)/DSP_Lib/Source/TransformFunctions/arm_rfft_fast_f32.c\
$(CMSIS)/DSP_Lib/Source/TransformFunctions/arm_rfft_fast_init_f32.c\
$(CMSIS)/DSP_Lib/Source/TransformFunctions/arm_rfft_init_f32.c

CMSIS_OBJS = $(CMSIS_SRCS:.c=.o)

# ---------------------------------------------------------------------------

# Codec 2

CODEC2_SRC=../src
CODEC2_SRCS=\
$(CODEC2_SRC)/codebooknewamp1.c \
$(CODEC2_SRC)/codebooknewamp1_energy.c \
$(CODEC2_SRC)/codec2.c \
$(CODEC2_SRC)/fdmdv.c \
$(CODEC2_SRC)/ofdm.c \
$(CODEC2_SRC)/filter.c \
$(CODEC2_SRC)/interldpc.c \
$(CODEC2_SRC)/mpdecode_core.c \
$(CODEC2_SRC)/gp_interleaver.c \
$(CODEC2_SRC)/varicode.c \
$(CODEC2_SRC)/codec2_fft.c \
$(CODEC2_SRC)/fifo.c \
$(CODEC2_SRC)/freedv_api.c \
$(CODEC2_SRC)/kiss_fft.c \
$(CODEC2_SRC)/kiss_fftr.c \
$(CODEC2_SRC)/mbest.c \
$(CODEC2_SRC)/newamp1.c \
$(CODEC2_SRC)/nlp.c \
$(CODEC2_SRC)/pack.c \
$(CODEC2_SRC)/phase.c \
$(CODEC2_SRC)/postfilter.c \
$(CODEC2_SRC)/quantise.c \
$(CODEC2_SRC)/sine.c

CFLAGS += -I../src -Isrc

# ---------------------------------------------------------------------------

# Libraries to link

LIBS = -g -lm

all: libstm32f4.a ofdm.bin

libstm32f4.a: $(CMSIS_OBJS) $(STM32F4LIB_OBJS)
	find $(PERIPHLIBDIR) -type f -name '*.o' -exec $(AR) crs libstm32f4.a {} ";"

# Rule for building .bin files from a .elf
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

# Rule for programming the modem hardware
%.pgm: %.bin
	$(SUDO) dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D $<

# ---------------------------------------------------------------------------

OFDM_SRC=src
OFDM_SRCS=\
$(OFDM_SRC)/ofdm_main.c \
$(OFDM_SRC)/tone.c \
$(OFDM_SRC)/sfx.c \
$(OFDM_SRC)/sounds.c \
$(OFDM_SRC)/morse.c \
$(OFDM_SRC)/menu.c \
$(OFDM_SRC)/tot.c \
$(OFDM_SRC)/ofdm_leds_switches.c \
$(OFDM_SRC)/system_stm32f4xx.c \
$(OFDM_SRC)/stm32f4_vrom.c \
$(OFDM_SRC)/stm32f4_dac.c \
$(OFDM_SRC)/stm32f4_adc.c

OFDM_ASM = $(OFDM_SRC)/startup_stm32f4xx.s
OFDM_ASM_OBJS = $(OFDM_ASM:.s=.o)

OFDM_SRCS += $(CODEC2_SRCS)
OFDM_OBJS = $(OFDM_SRCS:.c=.o)

.c.o:
	$(CC) $(CFLAGS)  $^ -c -o $@

.s.o:
	$(CC) $(CFLAGS)  $^ -c -o $@

ofdm.elf: $(OFDM_OBJS) $(OFDM_ASM_OBJS) libstm32f4.a
	$(CC) $(CFLAGS) -O3 $^ -o $@ $(LIBS)

# ---------------------------------------------------------------------------

clean:
	rm -f *.elf *.bin
	rm -f libstm32f4.a
	find . ../src -type f -name '*.o' | xargs rm -f
