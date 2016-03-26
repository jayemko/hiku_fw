/**
 * \file
 *
 * \brief DAC Sinewave Example.
 *
 * Copyright (c) 2011-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage DAC Sinewave Example
 *
 * \section Purpose
 *
 * The DAC Sinewave example demonstrates how to use DACC peripheral.
 *
 * \section Requirements
 *
 * This example can be used on any SAM3/4/V71/E70 boards.
 *
 * \section Description
 *
 * This application is aimed to demonstrate how to use DACC in free running
 * mode.
 *
 * The example allows to configure the frequency and amplitude of output
 * sinewave. The frequency could be set from 200Hz to 3KHz, and the peak
 * amplitude could be set from 100 to 1023/4095 in regard to 10/12bit
 * resolution.
 *
 * The example can also generate a full amplitude square wave for reference.
 *
 * The output could be monitored by connecting the following PIN that is used to one
 * channel of an oscilloscope.
 * \copydoc dac_sinewave_example_pin_defs
 *
 * \section Usage
 *
 * -# Build the program and download it into the evaluation board.
 * -# On the computer, open and configure a terminal application
 *    (e.g., HyperTerminal on Microsoft Windows) with these settings:
 *   - 115200 bauds
 *   - 8 bits of data
 *   - No parity
 *   - 1 stop bit
 *   - No flow control
 * -# In the terminal window, the following text should appear (values depend
 *    on the board and chip used):
 *    \code
	-- DAC Sinewave Example xxx --
	-- xxxxxx-xx
	-- Compiled: xxx xx xxxx xx:xx:xx --
	-- Menu Choices for this example--
	-- 0: Set frequency(200Hz-3kHz).--
	-- 1: Set amplitude(100-4095).--
	-- i: Display present frequency and amplitude.--
	-- m: Display this menu.--
\endcode
 * -# Input command according to the menu.
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "asf.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_dacc_sinewave_example.h"

#define DACC_ANALOG_CONTROL (DACC_ACR_IBCTLCH0(0x02) \
		| DACC_ACR_IBCTLCH1(0x02) \
		| DACC_ACR_IBCTLDACCORE(0x01))

//#define SINE	1

/** The maximal sine wave sample data (no sign) */
#define MAX_DIGITAL   (0x7ff)
/** The maximal (peak-peak) amplitude value */
#define MAX_AMPLITUDE (DACC_MAX_DATA)
/** The minimal (peak-peak) amplitude value */
#define MIN_AMPLITUDE (100)

/** SAMPLES per cycle */
#ifdef SINE
#define VOICESAMPLES (16)
#else
//#define VOICESAMPLES (500)
#define VOICESAMPLES (2860)
//#define VOICESAMPLES (10)
#endif

/** Default frequency */
#define DEFAULT_FREQUENCY 1000
/** Min frequency */
#define MIN_FREQUENCY   200
/** Max frequency */
#define MAX_FREQUENCY   3000

/** Invalid value */
#define VAL_INVALID     0xFFFFFFFF

#define STRING_EOL    "\r"
#define STRING_HEADER "-- DAC Sinewave Example --\r\n" \
		"-- "BOARD_NAME" --\r\n" \
		"-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

/** Current g_ul_index_sample */
uint32_t g_ul_index_sample = 0;
/** Frequency */
uint32_t g_ul_frequency = 0;
/** Amplitude */
int32_t g_l_amplitude = 0;

/** Waveform selector */
uint8_t g_uc_wave_sel = 0;
int g_once = 1;

volatile uint16_t g_uc_pdc_buffer[VOICESAMPLES];
/* PDC data packet for transfer */
pdc_packet_t g_pdc_dacc_packet;
/* Pointer to UART PDC register base */
Pdc *g_p_dacc_pdc;

#ifdef SINE
//sine wave - sox - 5khz - big endian (reversed byte)
//const int16_t gc_us_sine_data[VOICESAMPLES] = {0x0000,0x4B3C,0x79BB,0x79BB,0x4B3D,0x0000,0xB4C5,0x8645,0x8644,0xB4C4};
//const int16_t gc_us_sine_data[VOICESAMPLES] = {0x00,0x4B3C,0x79BC,0x79BC,0x4B3C,0x00,0xB4C4,0x8645,0x8645,0xB4C4};
const int16_t gc_us_sine_data[VOICESAMPLES] = {0x00,0x30FC,0x5A82,0x7640,0x7FFF,0x7641,0x5A82,0x30FB,0x00,0xCF05,0xA57E,0x89BF,0x8001,0x89BF,0xA57E,0xCF04};
#else
//hello female audacity @ 5kHz sampling (reversed byte)
const int16_t gc_us_sine_data[VOICESAMPLES] = {0x04,0x13,0xFFFF,0x01,0xFFFA,0x07,0x0B,0x20,0x39,0x24,0x19,0x0C,0xFFE9,0x06,0xFFFC,0x1E,0x22,0x18,0x1F,0x19,0x2C,0x13,0x4F,0x4B,0x19,0x2A,0x3A,0x31,0x2C,0x12,0xFFFA,0x14,0xFFF4,0x1B,0x03,0x0B,0xFFFF,0xFFC8,0xFFD3,0xFFE7,0xFFEA,0xFFF3,0xFFE4,0xFFF8,0xFFE1,0xFFD3,0xFFED,0xFFC3,0xFFCC,0xFFAA,0xFFC8,0xFFB6,0xFFBB,0xFFF1,0xFFF4,0xFFEF,0xFFD4,0xFFEB,0xFFDE,0xFFEE,0xFFF9,0xFFFD,0x26,0x1E,0x06,0x14,0xFFCF,0xFFBE,0xFFD5,0xFFD8,0xFFF7,0xFFF7,0xFFFC,0xFFF7,0x0B,0xFFEE,0xFFF0,0xFFF5,0xFFF2,0xFFD9,0xFFD1,0xFFD9,0xFFEC,0xFFF7,0xFFF6,0x10,0x0F,0x17,0x24,0x0C,0xFFE6,0xFFF3,0xFFF4,0x0F,0x1D,0xFFFF,0x08,0x07,0xFFD1,0xFFEB,0x0F,0x1A,0x28,0xFFE1,0x17,0xFFE8,0xFFD7,0x32,0xFFD1,0xFFD1,0xFFF6,0xFFD8,0xFFE7,0xFFE7,0xFFCC,0xFFCA,0xFFF3,0x0A,0xFFF8,0x11,0x0E,0xFFD7,0xFFFB,0xFFE2,0xFFF4,0x28,0x10,0x36,0x4D,0x19,0xFFF2,0x14,0x21,0x3F,0x2B,0x12,0x0C,0xFFF7,0x01,0x0B,0x12,0xFFEF,0xFFEF,0xFFEF,0xFFF7,0xFFFE,0xFFF7,0xFFF6,0xFFE1,0x1B,0xFFD7,0xFFDB,0xFFF6,0xFFF9,0x10,0x23,0x39,0x26,0x2D,0x26,0x1F,0x02,0xFFFE,0xFFF0,0xFFF1,0xFFF9,0xFFEB,0xFFF5,0x01,0xFFDB,0xFFE2,0x0D,0x0A,0x16,0x02,0xFFFE,0x26,0x24,0x25,0x31,0xFFFE,0xFFEF,0xFFF0,0x09,0xFFD6,0xFFD4,0xFFC7,0xFFB6,0xFFD7,0xFFF9,0xFFB8,0xFFF3,0x34,0xFFD6,0x31,0xFFE2,0xFFC0,0x1E,0x17,0x1D,0x40,0x23,0x48,0x4C,0x39,0x2D,0x1E,0x43,0x21,0x27,0xFFFC,0xFFD1,0x00,0x25,0xFFE8,0xFFF5,0xFFED,0x29,0x18,0xFFE0,0x02,0x36,0x54,0x21,0x23,0xFFF0,0x29,0x3A,0x14,0x1F,0xFFBD,0xFFDD,0x33,0x13,0x45,0xFFC9,0xFFB1,0xFFFF,0xFFCB,0xFFFE,0xFFE2,0xFFF8,0x06,0xFFDC,0xFFE3,0xFF7B,0xFFDE,0x55,0xFFA1,0x5D,0xFFF9,0xFF9F,0x72,0xFFF7,0x01,0xFFE5,0xFFCA,0x1C,0x0A,0x0B,0xFFE4,0xFF9C,0x21,0x00,0xFFA2,0xFFB5,0x2B,0x1B,0xFFFF,0x53,0xFF9F,0x26,0x8D,0xFFA4,0xFFF7,0xFFC9,0xFFBC,0x62,0x1C,0xFFAF,0xFFA1,0xFFCE,0x56,0x21,0x13,0xFF8C,0xFFD2,0x50,0x08,0x59,0xFFC2,0xFFDB,0x23,0xFFE1,0x61,0x74,0xFFA3,0x8F,0x28,0xFFD6,0x56,0xFFE8,0xFFCE,0x67,0x59,0xFFBB,0x34,0xFFAA,0x0D,0xFF81,0x7E,0x66,0xFF64,0x3E,0xFF8F,0xFFBA,0x14E,0x4A,0xFF59,0xAE,0xFFE6,0x59,0x7D,0xFEBF,0xFFEF,0xCE,0xFF6F,0xF1,0xFFB2,0xFF0E,0xFF,0xFF7F,0x42,0x0D,0xFF64,0xB3,0x9A,0x57,0xFFBC,0xFEC0,0xFF68,0xE6,0x120,0x10,0xFEE8,0x1A,0x94,0x0A,0x90,0xFF09,0x32,0x105,0xFF3C,0x97,0xFF7B,0xFFAC,0x57,0xFF04,0xDF,0x2A,0xFFD3,0x119,0xFFBA,0xFF74,0xB5,0xFFBD,0x5C,0x65,0xFED1,0xFF4F,0xFEEE,0xFF16,0x3A,0x3B,0xFFBF,0xFF1D,0xFEE3,0x53,0x05,0xE4,0xFFD3,0xFE58,0x17F,0xFFB4,0xFFDD,0xDB,0xFE5E,0xE7,0x88,0xFF95,0x7B,0xFF64,0xFF44,0xFF37,0x18C,0xFFF5,0xFEB1,0x90,0xFE04,0xC4,0x134,0xFF3A,0x17D,0xFF68,0xFFC0,0xFFA6,0x1A,0xE1,0xFEC7,0xFFF7,0x85,0x02,0xE9,0xFF11,0xFE75,0xA3,0x39,0x53,0xD0,0xFF72,0xFF82,0xEB,0xFF0F,0xFFAA,0x18,0xFFE9,0xFF56,0xFFEB,0xFF64,0xFFF7,0x3CC,0xFF0E,0xFE8A,0xFF94,0xFF55,0x16F,0x8E,0x4D,0xFE88,0xFE7A,0x27A,0x0A,0xFE85,0xFFEB,0xFDEC,0xFFA5,0x117,0xFF7D,0xFD96,0x41,0x102,0xFF06,0x1CA,0xFF36,0xFE6D,0x111,0xFF7A,0x207,0xB0,0xFD07,0xFE67,0xFEDD,0x1E3,0x1DB,0xFE30,0xFF29,0xFE88,0xDD,0x1BD,0x52,0x121,0xFF14,0x21C,0x149,0xFF68,0xFFBC,0xFEC6,0xA4,0x1E5,0x16F,0xFE41,0xFC87,0x32,0xFFFC,0x4A,0x339,0x32,0xFF04,0xFEC4,0xFFDD,0x1A6,0x134,0xFF07,0xFF4C,0xE1,0xB5,0x67,0xFE2C,0xFF6B,0x13B,0xFF82,0x138,0xDF,0xFC93,0xFF49,0x192,0xFF4C,0x125,0x120,0xFE39,0x16,0xA4,0xFE6D,0xFEAE,0x7F,0xFE8C,0xFC,0x314,0xFED2,0xFEBF,0xFE6D,0x70,0x24D,0x7B,0xFFC7,0xFF9D,0xFE59,0xFF75,0x166,0xFF6B,0x98,0xB5,0xFC8A,0xF6,0x187,0xFC2D,0x195,0x41,0xFAEE,0x218,0xD7,0xFDF8,0x492,0xFFFE,0xFE7F,0x2B2,0x193,0x269,0x70,0xFDB7,0xFF0D,0x1B9,0x13D,0xFE63,0xFD27,0xFC42,0xFEE6,0xFF56,0xFBCD,0xFC4E,0xFEAC,0x157,0x3EE,0x7F2,0x73E,0x381,0x3A2,0x3B1,0x3AA,0x358,0x1C5,0xFE74,0xFA12,0xF748,0xF5EF,0xF5B3,0xF806,0xF96B,0xF95C,0xFC83,0x145,0xC56,0x11DF,0xB0C,0x95D,0xB63,0xF1B,0xE53,0x82C,0xF6A2,0xE340,0xEA09,0xF259,0xF2F5,0xFD12,0xF8A4,0xEE3E,0xF81A,0xFFF6,0xFCD,0x1D97,0x111A,0xFF1,0x159F,0x12EE,0x1450,0xF679,0xD1C2,0xE6BE,0xFCAB,0xFB8F,0x80C,0xFA23,0xDF22,0xE4CF,0xF28D,0x524,0x1E33,0x232F,0x112B,0xC97,0x16FC,0x1437,0x1737,0xF539,0xC665,0xE10F,0xBB,0xFE20,0xE12,0xA1D,0xE584,0xDE33,0xDC48,0xF487,0x244E,0x2359,0x1598,0xFB4,0xD2A,0x1150,0x184F,0x1160,0xD805,0xC765,0xF47B,0xFD67,0x26E,0x1BF3,0x8CE,0xE267,0xD92F,0xDB7E,0xFC51,0x22B9,0x21AE,0x153B,0xF5C,0x1237,0x1125,0x1750,0x29C,0xC67B,0xD36E,0xCE,0xF9,0xC4B,0x1FB6,0x11F,0xDB0E,0xD6C9,0xDB08,0x99,0x25C3,0x229E,0x146D,0x1315,0x15E3,0x146D,0x172E,0xE70B,0xBE32,0xE8AE,0x687,0x3A8,0x171E,0x1D30,0xF4CC,0xD8D2,0xD524,0xD8CD,0x609,0x2924,0x21B1,0x1621,0x1916,0x17F6,0x17E1,0x1398,0xD90E,0xBDC0,0xEEAA,0x7AD,0x4B0,0x1981,0x1EA2,0xF469,0xCF46,0xD053,0xE525,0x788,0x21EB,0x24E3,0x1985,0x19B5,0x1CE1,0x1ED7,0x6E8,0xC89A,0xC4A9,0xF40F,0x2BA,0x7E6,0x203B,0x1E62,0xF353,0xC838,0xC98B,0xEF74,0xC4D,0x1A9E,0x246E,0x2118,0x1DDF,0x20C4,0x2337,0xF55A,0xBB56,0xCD14,0xF7EB,0xFE20,0x778,0x22D0,0x207A,0xF3B1,0xC7AC,0xD095,0xF16A,0x89,0x12F1,0x2783,0x2920,0x2493,0x2BBE,0x2574,0xE5A1,0xB42B,0xCF42,0xF363,0xF637,0x650,0x29A3,0x27C7,0xF5E9,0xC938,0xD388,0xED41,0xF6AA,0xF3E,0x2BEB,0x31A8,0x2E1D,0x3849,0x20C7,0xD18E,0xAEAD,0xD249,0xEE14,0xF052,0xBE2,0x32AE,0x2974,0xED4E,0xC9E3,0xDDF8,0xEC42,0xF1E4,0x1096,0x3288,0x37E7,0x3A87,0x3C35,0x428,0xBAF3,0xB5BA,0xD928,0xE73F,0xF170,0x19D4,0x3707,0x19DD,0xE327,0xD6AC,0xE5AE,0xE702,0xF415,0x1B6E,0x37CA,0x3D70,0x4640,0x2CAE,0xE099,0xB3C4,0xC43A,0xDABB,0xE112,0xFC75,0x29D5,0x3087,0x576,0xE215,0xE2B0,0xE5F7,0xE7EA,0x28F,0x28E5,0x3B6B,0x4843,0x410C,0x277,0xC40B,0xBD97,0xD0FC,0xD71C,0xE781,0x14C7,0x2F05,0x15D1,0xF4AE,0xEF5C,0xECC4,0xE3B1,0xF2AE,0x1ADC,0x34E8,0x425B,0x4433,0x16C5,0xD892,0xC1D6,0xCD3A,0xD460,0xDDAF,0x15B,0x2503,0x1D95,0xFCBA,0xF445,0xF41B,0xE7D2,0xEBA7,0xF19,0x2EBC,0x3EC6,0x4495,0x2236,0xE794,0xC91B,0xCBB0,0xD206,0xD93C,0xF572,0x1B76,0x1D21,0xFB,0xF90E,0xFBAD,0xED8B,0xE852,0x6AE,0x2A53,0x3B8B,0x4138,0x264A,0xF2AC,0xD1AD,0xCCED,0xD0B7,0xD77F,0xEF86,0x1038,0x1844,0x526,0xFBAA,0xFD70,0xF51E,0xEF59,0x52D,0x2508,0x3AF3,0x4191,0x2066,0xEE7E,0xD771,0xD4E8,0xD18D,0xD4E9,0xF032,0xE92,0x1007,0x98,0xFF13,0x189,0xF601,0xF203,0xA7E,0x2827,0x3BBB,0x3D40,0x17E8,0xEB4F,0xD92E,0xD695,0xD236,0xD7B3,0xF361,0xBD4,0xA31,0xFFB6,0x1C5,0x246,0xF7FD,0xF864,0x1126,0x2B62,0x3EA4,0x3758,0x9B2,0xE49E,0xDB8C,0xD826,0xD0A0,0xDB63,0xFA1E,0xAEF,0x31A,0xFE96,0x60C,0x2C8,0xF6B4,0xFF5E,0x1B16,0x3184,0x3EB8,0x2A76,0xFBBE,0xE078,0xDB9F,0xD630,0xD11A,0xE2DB,0xFFA0,0x748,0xFF07,0x1B2,0x8A4,0x10,0xF772,0x900,0x24CC,0x38D9,0x399D,0x15DF,0xEF2E,0xE03A,0xDA9F,0xD218,0xD47F,0xEDF5,0x3BC,0x134,0xFCD8,0x67B,0x892,0xFC1E,0xFD2C,0x1604,0x2F2B,0x3DC4,0x2B64,0x08,0xE6F3,0xE032,0xD76A,0xCF93,0xDE8A,0xF7C3,0xD0,0xFE18,0x3C3,0xBA7,0x4AC,0xFB81,0x9DA,0x23E2,0x383F,0x378E,0x12F7,0xEF27,0xE388,0xDE16,0xD2CB,0xD509,0xEB27,0xFBFA,0xFD0C,0xFFDB,0xAEC,0xB17,0xFFA4,0x3B6,0x1B3B,0x31AA,0x3B4D,0x2095,0xF761,0xE633,0xE17C,0xD689,0xD220,0xE3BE,0xF649,0xFA44,0xFCD0,0x949,0xD96,0x26B,0x57,0x1589,0x2BB8,0x3994,0x2B5F,0x2CB,0xE9C3,0xE34A,0xDAB3,0xD222,0xDDC9,0xEF61,0xF64F,0xFB00,0x6B9,0xE52,0x75D,0x232,0x1271,0x2787,0x3869,0x30EF,0x640,0xE6F6,0xE6EF,0xDEA4,0xD555,0xDCF2,0xE9C8,0xFAEB,0xF7E7,0x1F8,0x117B,0x6B5,0xFAC1,0x1158,0x3002,0x3D06,0x35AF,0x13F,0xD732,0xD32A,0xD8FC,0xDF4F,0xF563,0x555,0xFD67,0xF44A,0xF72E,0xFB1B,0xF95C,0xFD63,0x19D8,0x38E9,0x4A77,0x4643,0xFF52,0xB815,0xBA8B,0xD3A7,0xE350,0x6E7,0x1B84,0x4F9,0xEF87,0xF076,0xF1AB,0xF290,0x19C,0x283D,0x488D,0x566E,0x3B0D,0xDB54,0xA10E,0xBC11,0xDA9A,0xF715,0x2510,0x2031,0xF275,0xE0AC,0xE72E,0xEF06,0xFCBB,0x1792,0x3B6B,0x5273,0x544A,0xF02,0xA784,0xA274,0xD49C,0xED7A,0x1BF6,0x3753,0x8D,0xD5DB,0xDCFF,0xE4BC,0xF766,0x15E4,0x2D89,0x456A,0x5878,0x39BB,0xC921,0x8D79,0xC40C,0xECA9,0xCAB,0x3C72,0x1AEF,0xD451,0xCEEC,0xDE84,0xF164,0x1436,0x28D1,0x3950,0x4E1B,0x4ACB,0xF4D9,0x932B,0xAC68,0xEA9D,0x381,0x3615,0x3113,0xD6E3,0xC1AC,0xDBEE,0xE915,0x1332,0x2F91,0x2EE2,0x4187,0x5019,0xFA6,0xA09B,0xA269,0xE77D,0xFEBC,0x2B73,0x3B09,0xE4B8,0xB5EE,0xD649,0xE8F3,0xCEF,0x337F,0x321C,0x3708,0x4DAF,0x1ED5,0xA8CA,0x9BD9,0xE906,0x136,0x256D,0x4481,0xE542,0xA664,0xDA45,0xEC23,0x7DE,0x3A4F,0x2FFA,0x2C47,0x4B61,0x252E,0xB1E0,0x9FD8,0xEA95,0xFF8D,0x2187,0x3F81,0xE73C,0xAA3B,0xD280,0xEB31,0xFF6,0x3BC5,0x3100,0x2BC2,0x44E4,0x2062,0xAFBC,0xA237,0xF1EF,0x735,0x2414,0x3C5A,0xDCB3,0xA345,0xD815,0xEE82,0x1177,0x40BE,0x2E0B,0x2852,0x463F,0x1837,0xA811,0xAA9C,0xF93E,0x6C9,0x2C71,0x3402,0xCDFA,0xA831,0xDAC6,0xF341,0x1BD3,0x3F3D,0x2D0B,0x2C0B,0x3FCF,0x576,0x9E97,0xBB09,0x530,0xE68,0x31D3,0x2A39,0xB761,0xA511,0xE868,0xFBC5,0x28DA,0x42BE,0x248C,0x2C1D,0x3C81,0xE611,0x980D,0xD877,0xE83,0x110F,0x3D35,0x8C4,0x9E9D,0xC010,0xEFC8,0x64B,0x3A1C,0x35AE,0x205B,0x3952,0x276A,0xBD3E,0xA47C,0xF7C1,0xE11,0x219B,0x3907,0xDB63,0x9D3A,0xD68C,0xF6D6,0x191F,0x3F88,0x28BC,0x251B,0x3E1B,0x218,0x9EA7,0xC4FD,0xDA9,0x1103,0x3303,0x1FCE,0xACAE,0xA704,0xF0FE,0x6E8,0x2EE8,0x3DF1,0x1E31,0x2D5E,0x2E77,0xC61B,0x9E6E,0xFC70,0x16F8,0x1C6B,0x38D0,0xDA5F,0x98FD,0xDBE0,0xFC8F,0x1AB5,0x3F67,0x26C1,0x21BC,0x3942,0xF545,0x9CDE,0xD504,0x1772,0x14AE,0x3602,0xF4B,0x9A7E,0xB638,0xFA6C,0xB27,0x3D52,0x38DF,0x16A5,0x3205,0x1492,0xA3B2,0xBA56,0x18C0,0x14FC,0x2C02,0x255A,0xAE4D,0xA4AC,0xF51B,0x8BF,0x29E0,0x3DC0,0x1FB2,0x28FA,0x231B,0xBA28,0xA563,0x914,0x1EA1,0x2088,0x3423,0xCA0B,0x86B1,0xEBC2,0x13BE,0x1DDD,0x447C,0x24AA,0x1C2B,0x2877,0xC528,0x99FB,0x97F,0x2591,0x1EDA,0x2A75,0xD075,0x9D18,0xE0AD,0xF36,0x20BF,0x3792,0x2B69,0x2843,0x1EAA,0xC678,0xA083,0xFC4C,0x27B3,0x25F3,0x301C,0xCECF,0x9134,0xE27E,0x10E1,0x24CB,0x4107,0x297A,0x2508,0x1B5D,0xB280,0xA13D,0xF73,0x295D,0x287E,0x2869,0xBDD4,0x9A5E,0xF42B,0x10D4,0x2099,0x3E4F,0x28FD,0x26F3,0xFE2,0xA872,0xAE30,0x18ED,0x292E,0x294C,0x1480,0xB608,0xADAF,0xF933,0x19C4,0x2B80,0x37C4,0x2EC7,0x2A8B,0xE058,0x9010,0xD8D2,0x2A90,0x2B0F,0x3375,0xF4EB,0x96FE,0xC6F6,0x1039,0x1AAE,0x3495,0x3481,0x26BC,0x1C85,0xC122,0x9671,0xFDA5,0x319E,0x2CA8,0x1E65,0xC816,0xA932,0xE919,0x1808,0x2E25,0x35ED,0x2D27,0x2AC3,0xE0CC,0x8D6A,0xD575,0x2AE8,0x2EE1,0x31DE,0xF5C8,0x990D,0xC895,0x1855,0x1D7B,0x306F,0x3777,0x2C06,0x7D9,0xAA0B,0xA782,0xCC5,0x31D1,0x30EC,0xCC3,0xBEEA,0xB961,0xF7B5,0x1E72,0x2E78,0x32BB,0x37DC,0x1948,0xAA71,0x9955,0x3A0,0x2B86,0x3289,0x3274,0xC2A4,0x9795,0xF74D,0x2112,0x2669,0x36EF,0x3188,0x2005,0xC9E3,0x8FE8,0xE9AE,0x2F2E,0x2C1A,0x1BEA,0xE658,0xB812,0xE254,0x1BC3,0x26C0,0x2EB7,0x3F0A,0x242B,0xB28A,0x8E1E,0xF765,0x2D8A,0x2E4F,0x3266,0xDA4B,0x95BB,0xED43,0x31E1,0x2273,0x286D,0x40D8,0x2498,0xACE5,0x8D84,0xFA84,0x2F66,0x24F6,0x1BF6,0xE60D,0xBA2F,0xEFA9,0x24C3,0x231D,0x2C7B,0x4593,0x10A9,0x9827,0x9CF3,0xE1A,0x2C31,0x2DE5,0x25EB,0xBEBA,0xA65A,0x125A,0x372C,0x17F3,0x2A37,0x4B6A,0xFADB,0x8679,0xB732,0x1DD0,0x21E2,0x193D,0x137D,0xD545,0xC9FD,0x121A,0x2B91,0x1BE5,0x391A,0x3D47,0xC7FB,0x82A6,0xE56A,0x2A83,0x202D,0x272B,0xF1DB,0xA9AF,0xEA5E,0x3F5D,0x24C1,0x10AD,0x493A,0x243D,0x9213,0x9992,0x1AAF,0x28C7,0x554,0x1551,0xEE8A,0xC597,0x7C3,0x3940,0x1B74,0x22C2,0x4115,0xDA1D,0x8000,0xDEC8,0x3308,0x1B41,0x122A,0xFB57,0xB5B7,0xE1A0,0x42B7,0x3081,0x7C2,0x3B22,0x2805,0x9185,0x94BF,0x2249,0x309F,0xF983,0x341,0xF417,0xDDB0,0xBF4,0x3194,0x215E,0x2266,0x229C,0xC06E,0x9841,0xF596,0x32DE,0x1CFF,0xFC52,0xE250,0xD5A8,0x19C,0x3000,0x25D5,0x1D74,0x386B,0xEE6D,0x889E,0xC78D,0x2D6F,0x1D0D,0xF961,0xFF7F,0xF30A,0xF713,0x14DE,0x2346,0x2051,0x278B,0xF455,0xA944,0xC890,0x1512,0x264D,0xAFB,0xEC47,0xD7E1,0xF843,0x2647,0x246C,0x132C,0x2C5A,0x154A,0xACA6,0xA81C,0xDD6,0x292B,0xFD50,0xF92E,0xFC04,0xFA35,0xCE4,0x2036,0x2027,0x20D1,0x29D,0xBD53,0xC008,0xBB,0x2577,0x1770,0xEB44,0xD2B8,0xFD1C,0x2932,0x1F39,0xFD0,0x2A27,0x1DEA,0xB9CB,0xA44A,0x05,0x2779,0x739,0xFB4E,0xF7EA,0xF953,0x93F,0x1BD4,0x2239,0x21B0,0xB42,0xCB8E,0xB90D,0xEEDE,0x1BFF,0x16CC,0x39,0xE5B9,0xEEFF,0x1767,0x2234,0x146C,0x1F51,0x1E9A,0xD15C,0xA7E1,0xE86D,0x250F,0x136E,0xF4EC,0xF18D,0x2E6,0x13A1,0x12D1,0x1474,0x2428,0x14E6,0xCCAF,0xB7B8,0xF1FF,0x1DDC,0x10AE,0xF572,0xEB50,0xFB90,0x16D9,0x1BF5,0x141D,0x1BBC,0x195E,0xDBA7,0xB1C4,0xE2DA,0x1D39,0x1349,0xF853,0xF47F,0x159,0xF65,0x12A4,0x165B,0x1E8C,0x179B,0xDF37,0xB9E5,0xE01A,0x14ED,0x1C38,0xFD0B,0xE4D7,0xF4D3,0x1638,0x1BB9,0x1159,0x17D4,0x222E,0xF288,0xB3FF,0xCBE0,0xD68,0x1D1C,0x443,0xF31E,0xFB70,0xCFB,0xEA7,0x1094,0x1A67,0x2091,0xFCD2,0xC45F,0xC8FE,0xFE50,0x1F5D,0xD04,0xEB1B,0xEACD,0xC93,0x1B43,0x1053,0xF33,0x2190,0x1519,0xD2BC,0xB64D,0xEA22,0x1DEB,0x128C,0xF41C,0xF24D,0x8F9,0x1271,0x8A5,0xCC3,0x1EEA,0x1F1A,0xED4B,0xBEE6,0xD68A,0x94F,0x185F,0x5CC,0xF06F,0xF2F2,0xAF1,0x15DA,0xFFB,0x1074,0x2030,0x121E,0xD511,0xBAE4,0xEA54,0x1DBE,0x1426,0xF0D8,0xEE5F,0x9FD,0x15C1,0x839,0x70D,0x1CE5,0x25AC,0xF693,0xBFD8,0xCE92,0x75C,0x1FB4,0x500,0xE91F,0xF1E1,0xBCC,0x1146,0x900,0xDF1,0x20E8,0x210B,0xEDC9,0xBDE7,0xD4A8,0xD41,0x1C47,0xFFB0,0xED1D,0xFA61,0xE98,0xC54,0x50D,0x1055,0x21BB,0x1BA8,0xEB4F,0xC446,0xDA80,0xC6E,0x1B86,0x1CA,0xEBA5,0xF26F,0x676,0xD8D,0xC6A,0xFF5,0x1B31,0x1EF5,0xFABA,0xC9EF,0xCEEA,0xFE5C,0x1A4D,0xABC,0xF078,0xF29D,0x60E,0xC7B,0x7B2,0xB0F,0x163E,0x1E4C,0x8A3,0xDB36,0xCE55,0xF052,0x15AD,0x149C,0xF3DE,0xE4AB,0xF94E,0xE69,0xD4F,0x609,0xB89,0x1D63,0x21AC,0xFA39,0xCC1E,0xD448,0x2B2,0x183E,0x4C1,0xEF3B,0xF45F,0x40A,0x703,0x373,0x825,0x13E0,0x1ED1,0x15AF,0xF01D,0xD480,0xE29B,0x3EF,0x10DA,0x3B9,0xF036,0xEF21,0xFD09,0x79D,0x95E,0x941,0xFA3,0x1C3A,0x1C83,0xFBE2,0xD740,0xDB65,0xFEF4,0x11DF,0x4B9,0xEE9F,0xEC5F,0xFC59,0x6FF,0x68F,0x542,0xB7D,0x1A7C,0x248F,0xF41,0xE50C,0xD4BA,0xECEA,0x8D4,0x8DF,0xF57A,0xEBDC,0xF71B,0x2F2,0x245,0xFFBD,0x6DE,0x161B,0x24CB,0x22C7,0x43D,0xE07D,0xDA48,0xF1AF,0x506,0x15A,0xF491,0xF010,0xF680,0xFCD9,0xFF9A,0x4FE,0xD18,0x169C,0x22DF,0x2719,0x1117,0xE91F,0xD448,0xE2D0,0xFC15,0x2A8,0xF6A8,0xEFCF,0xF8BC,0x242,0xFC,0xFEDC,0x645,0x14AF,0x2240,0x280F,0x1CBF,0xFEBF,0xDFA2,0xD6E3,0xE4FB,0xF720,0xFE3D,0xF973,0xF63F,0xFBCC,0x273,0x3A6,0x35A,0x7FE,0x13B2,0x204B,0x2618,0x1BCF,0xFF36,0xDF58,0xD325,0xDF56,0xF21C,0xFD5D,0xFF7B,0xFFC4,0x189,0x1F6,0x6A,0xC0,0x4F6,0xD2E,0x1775,0x1F33,0x1F72,0x1045,0xF2CA,0xD841,0xD358,0xE372,0xF94E,0x4C5,0x549,0x426,0x487,0x317,0xFDCC,0xFACC,0x45,0xD02,0x18AE,0x1E4B,0x1C22,0xDE0,0xF513,0xDD66,0xD56A,0xE1E2,0xF7D8,0x612,0x7DE,0x57B,0x384,0xD6,0xFB97,0xF6F1,0xFB23,0x82C,0x1648,0x1DCE,0x1E47,0x169E,0x383,0xEA5B,0xD809,0xD8D5,0xEB04,0xFF53,0x959,0x969,0x5C0,0x151,0xFCA3,0xF858,0xF82C,0x1C,0xD78,0x18FC,0x1D87,0x1C1B,0x130F,0xFFD5,0xE876,0xD931,0xDB9B,0xECAC,0xFF9D,0x9BC,0xA3B,0x6F3,0x315,0xFDB5,0xF85C,0xF694,0xFCBC,0x91C,0x14F2,0x1B1C,0x1AC3,0x156A,0x701,0xF148,0xDE6A,0xD9FE,0xE754,0xFB96,0x903,0xB39,0x7A1,0x2CE,0xFE16,0xF96C,0xF5D9,0xF785,0xEA,0xD6A,0x1741,0x1A8F,0x178D,0x1037,0x12E,0xED34,0xDEBB,0xDE92,0xECB4,0xFEC5,0x99F,0x9B0,0x43C,0xFF63,0xFBEB,0xF89B,0xF6C8,0xF9BF,0x29A,0xE3A,0x167B,0x1817,0x1505,0xFBF,0x481,0xF2D9,0xE348,0xDF11,0xE99C,0xFB1A,0x6E6,0x8A9,0x519,0x182,0xFF30,0xFBF3,0xF7BC,0xF712,0xFDD4,0xA07,0x144A,0x17E5,0x1501,0x1066,0x9BE,0xFC32,0xEB1C,0xE08C,0xE4C7,0xF4A0,0x390,0x919,0x6C4,0x30A,0x41,0xFD06,0xF84A,0xF4FD,0xF7FA,0x189,0xD12,0x13FF,0x14EE,0x11C6,0xDC4,0x8FC,0xFE65,0xEF1E,0xE51E,0xE830,0xF571,0x22C,0x639,0x303,0xFF80,0xFE7F,0xFD45,0xFA23,0xF75D,0xF8E7,0xFF98,0x8A3,0xFB6,0x122D,0x112E,0xE95,0xBE4,0x5E2,0xF948,0xEC37,0xE776,0xEDE2,0xF9B0,0x181,0x29B,0xDE,0x3A,0x76,0xFEC4,0xFAD6,0xF826,0xFA71,0x15A,0x984,0xF8D,0x117B,0x1060,0xD6D,0xA18,0x448,0xF98E,0xEE76,0xEA55,0xEF5E,0xF943,0x9B,0x1D4,0xCA,0x29,0x1E,0xFE44,0xFA8B,0xF7E9,0xFA4E,0xE1,0x802,0xD2C,0xEC7,0xD6E,0xB12,0x91F,0x62A,0xFF81,0xF5CF,0xEE74,0xEEE5,0xF5AD,0xFD30,0x43,0xFF71,0xFEED,0xFFB7,0x11,0xFDF8,0xFAF5,0xF9FB,0xFCAC,0x217,0x7F8,0xC31,0xD37,0xBE3,0x9C7,0x7B1,0x511,0xFEFC,0xF672,0xF09D,0xF0E4,0xF616,0xFBB8,0xFEE4,0xFF4E,0xFF70,0x13,0xFFAB,0xFDFA,0xFB9A,0xFB98,0xFF23,0x56D,0xBB3,0xDB2,0xC18,0xA30,0x926,0x728,0x151,0xF907,0xF2A2,0xF177,0xF53E,0xFA6E,0xFDDE,0xFEF6,0xFF75,0xFFC7,0xFF75,0xFE0B,0xFBDA,0xFB14,0xFCFF,0x19F,0x7C0,0xBF5,0xC00,0x989,0x7A2,0x6CE,0x47E,0xFF4C,0xF98D,0xF6AA,0xF799,0xFA93,0xFCA9,0xFD0C,0xFCFB,0xFD7E,0xFE7E,0xFE06,0xFCEA,0xFC2A,0xFCB5,0xFEAD,0x10E,0x412,0x79E,0x965,0x8CA,0x6B4,0x528,0x4BE,0x34C,0x6F,0xFC9D,0xFA35,0xFA33,0xFBBF,0xFCB9,0xFC1D,0xFBAB,0xFCB2,0xFE24,0xFE73,0xFD48,0xFC32,0xFC85,0xFE6E,0x54,0x1D0,0x3D0,0x63A,0x794,0x51C,0x1A7,0x83,0x15E,0x26A,0x11A,0xFE72,0xFD3A,0xFE69,0x45,0xE3,0xFF1E,0xFCBE,0xFBEB,0xFD61,0xFF2A,0xFF7F,0xFE5E,0xFD60,0xFE65,0x0D,0x111,0xF9,0xDE,0x1F3,0x386,0x3EC,0x322,0x209,0x172,0x130,0x65,0xFF1B,0xFEAC,0xFF6C,0xFF9D,0xFF65,0xFEF0,0xFF3E,0xFFC2,0xFF6B,0xFEBF,0xFE47,0xFE57,0xFF0F,0xFFFD,0x39,0x1F,0x55,0xED,0x151,0x185,0x12C,0xAF,0x58,0x59,0x81,0x54,0xFFD2,0xFF5E,0xFF77,0xFF82,0xFFCA,0x0A,0x28,0x2B,0xFFCF,0xFF74,0xFFA8,0x17,0x3E,0x2A,0x32,0xC3,0x130,0x14E,0xB0,0xFFBC,0xFFB6,0x0E,0x55,0x22,0xFFA4,0xFF81,0xFFB4,0x35,0x0A,0xFF90,0xFF68,0x38,0xD7,0x96,0x18,0xFF7D,0xFF72,0xFFD7,0x49,0x40,0xFFE9,0xFFCA,0x21,0x1F,0x20,0xFF2E,0xFE86,0xFED9,0xFFD6,0x75,0x46,0xFFA2,0xFEF8,0xFF23,0xFFB9,0x90,0x72,0xFFFE,0xFFE7,0x2C,0x7B,0xB0,0x62,0xFF94,0xFF4F,0xFF6B,0xFFA3,0xFFBC,0xFFC1,0xFFE0,0x30,0x1C,0x03,0x14,0xFFF6,0xFFCD,0xFFE3,0x02,0x39,0x6B,0x29,0xFFDE,0xFFEC,0x67,0xBD,0x31,0xFF3E,0xFF3F,0x1C,0xB5,0x87,0xFF98,0xFF69,0x4F,0xC2,0x71,0xFFED,0xFFB8,0xFFF0,0x50,0x21,0xFFAA,0xFF92,0x09,0x34,0xFFF4,0xFFB6,0xFFA1,0x33,0x56,0xFFD7,0xFFD4,0x31,0x49,0x14,0xFF86,0xFF7C,0x0A,0x4D,0x42,0xFFBA,0xFF82,0xFFBB,0xFFDA,0xFFB5,0xFF81,0xFFE0,0x1E,0x8A,0xC5,0x3A,0xFFED,0x06,0xFFC5,0xFFB8,0xFFA6,0xFFCD,0x41,0x42,0xFFD8,0xFF89,0xFF6C,0xFFE8,0x1C,0xFFCE,0xFF59,0xFF7E,0x15,0x6E,0x4C,0xFFE5,0xFFBA,0xFFA8,0xFFDB,0x01,0xFFFF,0xFFA8,0xFFA0,0xFFCE,0x27,0xFFFA,0xFFC5,0xFFBB,0xFFE1,0x35,0x12,0xFFFC,0x0E,0xFFF9,0xFFEF,0xFFF5,0xFFAD,0xFFE0,0x17,0x1A,0x28,0x29,0x11,0xFFCE,0xFF8A,0xFFB1,0xFFF7,0x33,0x3F,0x14,0xFFAD,0xFFD8,0x2A,0x58,0x1F,0xFFAF,0xFF84,0xFFF9,0x5A,0x4A,0x10,0xFFD5,0xFFC3,0xFFA3,0xFFC7,0xFFD1,0x02,0xFFCC,0xFFF5,0xFF8B,0x0D,0x03,0x58,0xFF80,0xFFC3,0xFE01,0xFCED,0x66,0xFFBE,0xFE52,0xFC04,0xFD2F,0xFDFB,0xFD69,0xFDB5,0x1D,0x314,0x418,0x3D0,0x225,0x28,0xFE88,0xFE73,0xFF4E,0xFFC4,0xFF1E,0xFF58,0xC7,0x175,0xFFED,0xFFCB,0xFFCE,0xDE,0x163,0x195,0x148,0x66,0xFFF8,0xFFA6,0xFFD8,0xFF62,0xFF48,0xFF70,0x04,0x31,0xA3,0x100,0xF6,0x53,0xFFEC,0x81,0x130,0xCA,0xFFF8,0xFFAA,0x83,0xAC,0xFFDE,0xFF42,0xFFA8,0xBE,0x129,0xE1,0x40,0x65,0xAE,0xA8,0x1E,0xFFBF,0xFFD9,0x82,0x92,0x05,0xFFBB,0xFFDF,0x1A,0x4D,0xFFFE,0xFFD7,0x4B,0x96,0x81,0x36,0x0E,0xFFE0,0x17,0x59,0x3C,0xFFE0,0xFFCD,0xFFF5,0x91,0x82,0x0E,0xFFD0,0xFF7A,0xFFD6,0x19,0xFFFC,0xFF1C,0xFEF9,0xFFAC,0x72,0x67,0xFF9E,0xFF5F,0xFFB5,0x3E,0x4C,0xFFF5,0xFF4C,0xFFE0,0x94,0x74,0xFFF3,0xFFAC,0xFFD0,0x0C,0x18,0xFF89,0xFF87,0xFFFB,0x36,0x61,0x94,0x50,0x0E,0xFFF8,0x56,0xD3,0x7F,0xFFD8,0xFF9D,0x56,0xFA,0xB0,0xFFE2,0xFF83,0x68,0x11E,0xE4,0x11,0x00,0x93,0xDE,0x8A,0xFFC8,0xFFD1,0x2E,0x16,0xFFC0,0xFFE4,0x35,0x72,0x1C,0xFFCB,0xFFFD,0x8D,0x47,0xFF76,0xFF8F,0x0C,0x5D,0xFFBF,0xFEFF,0xFF31,0xFFD4,0x0B,0xFFA5,0xFF37,0xFFA0,0x5A,0x47,0xFFB6,0xFF90,0xFFD8,0x0E,0xFFAF,0xFFB5,0xFFEC,0xFFF1,0xFFD7,0xFFA5,0xFFE3,0xFFFE,0xFFF1,0xFF87,0xFFA3,0xFFEE,0x43,0xFFE4,0xFF64,0xFFB2,0x11,0x31,0xFFC2,0xFFCD,0xFFE4,0x09,0x12,0x19,0xFFF3,0xFFE1,0xFFD0,0xFFEA,0x16,0x0C,0xFFB5,0xFF85,0x0A,0x5A,0x04,0xFF70,0xFF80,0x2E,0x63,0xFFE1,0xFF92,0xFFC2};

//const uint16_t gc_us_sine_data[VOICESAMPLES] = {0xFA0,0xFA0,0xFA0,0xFA0,0xFA0,0xFA0,0xFA0,0xFA0,0xFA0,0xFA0};
#endif

/** 100 points of sinewave samples, amplitude is MAX_DIGITAL*2 */
/*const int16_t gc_us_sine_data[SAMPLES] = {
	0x0,   0x080, 0x100, 0x17f, 0x1fd, 0x278, 0x2f1, 0x367, 0x3da, 0x449,
	0x4b3, 0x519, 0x579, 0x5d4, 0x629, 0x678, 0x6c0, 0x702, 0x73c, 0x76f,
	0x79b, 0x7bf, 0x7db, 0x7ef, 0x7fb, 0x7ff, 0x7fb, 0x7ef, 0x7db, 0x7bf,
	0x79b, 0x76f, 0x73c, 0x702, 0x6c0, 0x678, 0x629, 0x5d4, 0x579, 0x519,
	0x4b3, 0x449, 0x3da, 0x367, 0x2f1, 0x278, 0x1fd, 0x17f, 0x100, 0x080,

	-0x0,   -0x080, -0x100, -0x17f, -0x1fd, -0x278, -0x2f1, -0x367, -0x3da, -0x449,
	-0x4b3, -0x519, -0x579, -0x5d4, -0x629, -0x678, -0x6c0, -0x702, -0x73c, -0x76f,
	-0x79b, -0x7bf, -0x7db, -0x7ef, -0x7fb, -0x7ff, -0x7fb, -0x7ef, -0x7db, -0x7bf,
	-0x79b, -0x76f, -0x73c, -0x702, -0x6c0, -0x678, -0x629, -0x5d4, -0x579, -0x519,
	-0x4b3, -0x449, -0x3da, -0x367, -0x2f1, -0x278, -0x1fd, -0x17f, -0x100, -0x080

};*/

//sine wave - sox - 5khz - little endian (reversed byte)
//const int16_t gc_us_sine_data[VOICESAMPLES] = {0x0000,0x4B3D,0x79BB,0x79BB,0x4B3C,0x0000,0xB4C4,0x8644,0x8645,0xB4C4};

//since wave - audacity defaults - 5khz (reversed byte)
//const int16_t gc_us_sine_data[VOICESAMPLES] = {0x00,0x4B3B,0x79BD,0x79B9,0x4B3F,0xFFFE,0xB4C7,0x8643,0x8646,0xB4C3};


/**
* \brief Configure console.
*/
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};

	/* Configure console UART. */
	#if !SAM4L
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	#endif
	stdio_serial_init((Usart *)CONF_UART, &uart_serial_options);
}

/**
 * \brief Display main menu.
 */
static void display_menu(void)
{
	printf("\r\n");
	puts("======== Started DAC output ========\r");
}

/**
 * \brief SysTick IRQ handler.
 */
/*void SysTick_Handler(void)
{
	uint32_t status;
	uint32_t dac_val;
	
	status = dacc_get_interrupt_status(DACC_BASE);

	if ((status & DACC_ISR_TXRDY) == DACC_ISR_TXRDY) {
		g_ul_index_sample++;

		if (g_ul_index_sample >= VOICESAMPLES) {
			g_ul_index_sample = 0;
			g_once = 0;
		}
				
		//*	Convert 16bit signed integer to 12bit unsigned -> ((uint16_t)(audsample + 32768))>>4;
		dac_val = (gc_us_sine_data[g_ul_index_sample] + 0x8000) >> 4;
		
		//*	Print converted data for debug
		if (g_once == 1){
			printf("%i = %i\r\n", gc_us_sine_data[g_ul_index_sample], dac_val );
		}		

		//***** 5. Write conversion data with dacc_write_conversion_data().
		dacc_write_conversion_data(DACC_BASE, dac_val);
	}
}*/


/**
 * \brief Interrupt handler for the DACC
 */
void DACC_Handler(void)
{
   uint32_t status;
   status = dacc_get_interrupt_status(DACC_BASE);
   /* DACC_ISR_TXBUFE and DACC_ISR_ENDTX both indicates ending pdc buffer */
   /* If ready for new data */
//   if ((status & DACC_ISR_TXBUFE) == DACC_ISR_TXBUFE) {
   if ((status & DACC_ISR_ENDTX) == DACC_ISR_ENDTX) {

	  // printf("e\r\n");

         g_pdc_dacc_packet.ul_addr = (uint32_t) g_uc_pdc_buffer;
         g_pdc_dacc_packet.ul_size = VOICESAMPLES;
		 pdc_tx_init(g_p_dacc_pdc, &g_pdc_dacc_packet, 0);
		 pdc_enable_transfer(g_p_dacc_pdc, PERIPH_PTCR_TXTEN); 
   }
}


/**
 *  \brief DAC Sinewave application entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	uint8_t uc_key;
	uint32_t ul_freq, ul_amp;

	/* Initialize the system */
	sysclk_init();
	board_init();

	//	============================================
	//	Convert 16bit signed integer PCM data to 12bit unsigned integer DAC data
	//	============================================
	for (int i = 0; i< VOICESAMPLES; i++){
		g_uc_pdc_buffer[i] = (gc_us_sine_data[i] + 0x8000) >> 4;
	}



	//	============================================
	//	Configure the TC PIO
	//	============================================
	/** Configure PIO Pins for TC */
	ioport_set_pin_mode(PIN_TC_WAVEFORM, PIN_TC_WAVEFORM_MUX);
	/** Disable I/O to enable peripheral mode) */
	ioport_disable_pin(PIN_TC_WAVEFORM);

	/* Initialize debug console */
	configure_console();

	/* Output example information */
	puts(STRING_HEADER);
	printf("\r\n");
	
	//	============================================
	//	Configure the DACC
	//	============================================

	//***** 1. DACC clock should be enabled before using it.
	sysclk_enable_peripheral_clock(DACC_ID);
	
	//***** 2. Reset DACC registers
	dacc_reset(DACC_BASE);

	//***** 7. Initialize timing, amplitude and frequency */
	/* Half word transfer mode */
	dacc_set_transfer_mode(DACC_BASE, 0);

	//***** 6X. Configure trigger with dacc_set_trigger() and dacc_disable_trigger().
	//dacc_set_trigger(DACC_BASE, DACC_MR_TRGEN_EN | DACC_MR_TRGSEL(1));    /* Set trigger to TC0_0 on 8/16 kHz. Need to verify vaulue. */
	dacc_set_trigger(DACC_BASE, 2);	//(2 = TC0 Output Chan. 1)

	//***** 4. Initialize DACC timing with dacc_set_timing() (different DAC peripheral may require different parameters).
	dacc_set_timing(DACC_BASE, 0x08, 0, 0x10);

	//***** 11a. Disable TAG and select output channel DACC_CHANNEL */
	dacc_set_channel_selection(DACC_BASE, DACC_CHANNEL);



	//***** 11b. Enable output channel DACC_CHANNEL */
	dacc_enable_channel(DACC_BASE, DACC_CHANNEL);

	/* Set up analog current */
	dacc_set_analog_control(DACC_BASE, DACC_ANALOG_CONTROL);

	g_l_amplitude = 4095;//MAX_AMPLITUDE / 2;
	g_ul_frequency = DEFAULT_FREQUENCY;
	//SysTick_Config( sysclk_get_cpu_hz() / (5000) );
	printf("raw data (int16) = dac data (uint12)\r\n");
	
	// configure timer to generate DACC conversion at 5000hz so that timer will trigger DAC --> timer will generate after 5000 cycles (bytes) --> PDC will move data to dac 5000 times while CPU idling
	// configure PDC to transfer X bytes from RAM to DAC register -- > enable PDC --> enable DAC + PDC connection --> enable DAC output

	//	============================================
	//	Configure the DMA (PDC) for DACC transfer
	//	============================================	
	//***** 10X. If the DACC can work with PDC, use dacc_get_pdc_base() to get PDC register base for the DAC controller.
    /* Get pointer to DACC PDC register base */
    g_p_dacc_pdc = dacc_get_pdc_base(DACC_BASE);
    /* Initialize PDC data packet for transfer */
    g_pdc_dacc_packet.ul_addr = (uint32_t) g_uc_pdc_buffer;
    g_pdc_dacc_packet.ul_size = VOICESAMPLES;
    /* Configure PDC for data transmit */
    pdc_tx_init(g_p_dacc_pdc, &g_pdc_dacc_packet, NULL);
    /* Enable PDC transfers */
    pdc_enable_transfer(g_p_dacc_pdc, PERIPH_PTCR_TXTEN);
	
	//	============================================
	//	Configure the DACC interrupt
	//	============================================	
	//***** 8X. Control interrupts with dacc_enable_interrupt(), dacc_disable_interrupt(), dacc_get_interrupt_mask() and dacc_get_interrupt_status().
	/* Enable DACC + PDC IRQ */
	NVIC_DisableIRQ(DACC_IRQn);
	NVIC_ClearPendingIRQ(DACC_IRQn);
	NVIC_SetPriority(DACC_IRQn, 0);
	NVIC_EnableIRQ(DACC_IRQn);
	dacc_enable_interrupt(DACC, DACC_ISR_TXBUFE);	
	//dacc_enable_interrupt(DACC, DACC_ISR_ENDTX);
	
	//	============================================
	//	Configure the TC trigger for DACC conversion	
	//	============================================
	uint32_t ra, rc;
	/* Enable TC0 peripheral clock. */
	pmc_enable_periph_clk(ID_TC1);
	// Init TC to waveform mode.
	tc_init(TC0, 1,
		  TC_CMR_TCCLKS_TIMER_CLOCK1	/* Waveform Clock Selection MCK/2 */
		| TC_CMR_WAVE					/* Waveform mode is enabled */
		| TC_CMR_ACPA_SET				/* RA Compare Effect: set */
		| TC_CMR_ACPC_CLEAR				/* RC Compare Effect: clear */
		| TC_CMR_WAVSEL_UP_RC			/* UP mode with automatic trigger on RC Compare */
	);	
	/* Configure waveform frequency and duty cycle. */
	rc = (sysclk_get_peripheral_bus_hz(TC0) / 2 / 5000);
	tc_write_rc(TC0, 1, rc);
	ra = (100 - 50) * rc / 100;
	tc_write_ra(TC0, 1, ra);
	/* Enable TC TC0_CHAN_0_WAVE. */
	tc_start(TC0, 1);

	/* Main menu */
	display_menu();

	while (1) {
	}
}
