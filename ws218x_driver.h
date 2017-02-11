/*
 * ws218x_driver.h
 *
 *  Created on: Feb 11, 2017
 *      Author: loki
 */
#include <msp430f5529.h>
#include <stdint.h>

#ifndef WS218X_DRIVER_H_
#define WS218X_DRIVER_H_

#define F_MCLK 24000000UL

// defines for internal routines
void init_clocks(void);
void init_spi(void);
void setup(void);

void ledbits2pulse(const uint8_t * const src, uint8_t *dst);
void sendGRB(const uint8_t * const color_data, unsigned byte_cnt);
void setvcoreup(unsigned int level);

#define USE_DMA_SPI         /* define to use DMA, undef to use polled SPI */
//#define USE_ASM_VERSION     /* define to use inline msp430 asm, undef to use C versions */
#define USE_32K_XTAL        /* define to use the external 32.768k XTAL instead of the REF clock */

#if !defined(__GNUC__) && defined(USE_ASM_VERSION)
#error msp430-elf-gcc is required to use the inline assembler version!
#endif

#define ANIMATION_FRAMES 4  /* how many frames are we animating */
#define LED_CNT 5           /* how many leds do you have */
#define BYTE_PER_LED 3      /* green 8 bits, red 8 bits, blue 8 bits 0-255 where 0 is dark and 255 bright*/

#define RGB(RED,GREEN,BLUE) (GREEN),(RED),(BLUE)




#endif /* WS218X_DRIVER_H_ */
