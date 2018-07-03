Starting off with a copy of the base codec2 "src" directory.

Here we find the define CORTEX_M4. This is used by the 1600 mode SM1000 device.
Therefore, it can't really be used for the 700D mode, although it does signify
that whole blocks of code can be ignored because they don't make sense in firmware.

What is needed is a new define, so I created STM700D. If this is defined, which
it is in the Makefile, then the code is needed for the OFDM firmware.

So, the "src" files may contain various combinations of CORTEX_M4 and STM700D.

There is also another define ARM_MATH_CM4 which is used to select the ARM math
functions over the regular C library.

All the code is designed to compile under a minimum of C99 standard. Any older
standard, and all bets are off. Which means also, that C11 is good also.

Everything in the "src" directory should be maintained to the original, so only
the changed files will be rolled back in to the main repository when the project
is finished.

Notes:

Mode 700D OFDM uses four Vocoder Frames (4 * 28) or 112 bits. FEC is another 112 bits
(LDPC). Thus, I have made the ADC/DAC buffers large enough to hold this.

OK, I deleted all the morse code menu stuff. Maybe we can add it back in when the
OFDM is working. Right now it is causing debugging problems.

OK, I got rid of 16K sample rate. Everything is 8K now.
