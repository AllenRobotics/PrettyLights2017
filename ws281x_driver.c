/*
 * main.c - msp430f5529 ws281x driver
 *
 * Desc:
 *     Drives ws281x (ws2811/ws2812b/neopixels) leds using SPI driven
 *     by DMA.  This code also provides an example of how to use inline
 *     asm with msp430-elf-gcc to create some utility functions.
 */

#include <msp430.h>
#include <msp430f5529.h>
#include <stdint.h>
#include "ws218x_driver.h"



uint8_t pulse_data[BYTE_PER_LED * LED_CNT * 8]; // buffer to hold spi pulse bits

void init_clocks(void)
{
    P1DIR |= BIT0;                          // ACLK set out to pins
    P1SEL |= BIT0;
    P2DIR |= BIT2;                          // SMCLK set out to pins
    P2SEL |= BIT2;
    P7DIR |= BIT7;                          // MCLK set out to pins
    P7SEL |= BIT7;

#if defined(USE_32K_XTAL)
    P5SEL |= BIT4|BIT5;                     // Select XT1
    UCSCTL6 &= ~(XT1OFF|XCAP_3);            // XT1 On, clear XCAP settings
    UCSCTL3 = SELREF__XT1CLK;               // FLL Reference Clock = XT1, default not actually needed
#else
    UCSCTL3 |= SELREF__REFOCLK;             // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA__REFOCLK;               // Set ACLK = REFO
#endif

    __bis_SR_register(SCG0);                // Disable the FLL control loop
    UCSCTL0 = 0x0000;                       // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_6;                    // Select DCO range 24MHz operation

#if F_MCLK == 6400000
    UCSCTL2 = FLLD_1 + 194 + 1;             // Set DCO Multiplier for ~12MHz
    #define DCO_DELAY 200000
#elif F_MCLK == 12000000
    UCSCTL2 = FLLD_1 + 365 + 1;             // Set DCO Multiplier for ~12MHz
    #define DCO_DELAY 375000
#elif F_MCLK == 16000000
    UCSCTL2 = FLLD_1 + 488 + 1;             // Set DCO Multiplier for ~16MHz
    #define DCO_DELAY 500000
#elif F_MCLK == 17120000
    UCSCTL2 = FLLD_1 + 522 + 1;             // Set DCO Multiplier for ~17.12MHz
    #define DCO_DELAY 535000
#elif F_MCLK == 19200000
    UCSCTL2 = FLLD_1 + 585 + 1;             // Set DCO Multiplier for ~17.12MHz
    #define DCO_DELAY 600000
#elif F_MCLK == 24000000
    UCSCTL2 = FLLD_1 + 730 + 1;             // Set DCO Multiplier for ~24MHz
    #define DCO_DELAY 750000
#else
#error Set a valid F_MCLK value
#endif
                                            // (N + 1) * FLLRef = Fdco
                                            // (522 + 1) * 32768 = ~17.12MHz
                                            // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 17.12 MHz / 32,768 Hz = 535000 = MCLK cycles for DCO to settle
    // delay above based used to compute alogrithm
    __delay_cycles(DCO_DELAY);

    // Loop until XT1,XT2 & DCO fault flag is cleared
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                  // Clear fault flags
    } while (SFRIFG1 & OFIFG);              // Test oscillator fault flag
}

void init_spi(void)
{
    /* Code is using UCB0 */
    /* configure UCB0SIMO on P3.0 */
    P3SEL |= BIT0;                          // Set P3.0 to be USC0MOSI (Master Out/Slave In)

    UCB0CTL0 |= (UCCKPH | UCMSB | UCMST | UCSYNC);  // 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;                   // SMCLK used as SPI clock
#if F_MCLK == 6400000
    UCB0BR0 |= 0x01;                        // 1/(6.4MHz/1) = ~0.15625us per bit
#elif F_MCLK == 12000000
    UCB0BR0 |= 0x02;                        // 1/(12MHz/2) = ~0.166us per bit
#elif F_MCLK == 16000000
    UCB0BR0 |= 0x03;                        // 1/(16MHz/3) = ~0.1875us per bit
#elif F_MCLK == 17120000
    UCB0BR0 |= 0x03;                        // 1/(17.12MHz/3) = ~0.175us per bit
#elif F_MCLK == 19200000
    UCB0BR0 |= 0x03;                        // 1/(19.2MHz/3) = ~0.15625us per bit
#elif F_MCLK == 24000000
    UCB0BR0 |= 0x04;                        // 1/(24MHz/4) = ~0.166us per bit
#endif
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;
}

void setup(void)
{
    setvcoreup(0x01);                       // crank up core power to max
    setvcoreup(0x02);
    setvcoreup(0x03);
    init_clocks();                          // configure UCS clocks
    init_spi();                             // setup spi mosi on P3.0 use SMCLK
}

/*int main(void)
{
    WDTCTL = WDTHOLD | WDTPW;

    setup();

    while (1) {
        unsigned frame_offset;

        // loop through each "animation frame" 3 bytes per led, 4 leds for a total of 12 bytes
        for (frame_offset = 0; frame_offset < BYTE_PER_LED * LED_CNT * ANIMATION_FRAMES;
            frame_offset += (BYTE_PER_LED * LED_CNT)) {
            sendGRB(&leds[frame_offset], BYTE_PER_LED * LED_CNT);
            __delay_cycles(500 * (F_MCLK / 1000));
        }
    }
}*/


/*
 * ledbits2pulse - create SPI pulse bits
 *
 * Convert each bit of an RGB color to a ws281x pulse for use with SPI
 * save in dst. Assumes user has provided enough room for dst which is 8x
 * the size of src.
 *
 * 0 pulse is ~350ns high, ~1000ns low
 * 1 pulse is ~700ns high, ~700ns low
 */
void ledbits2pulse(const uint8_t register * const src, uint8_t register *dst)
{
#if defined(USE_ASM_VERSION)
    unsigned register mask, work, p0, p1;

    __asm__ volatile (
        " mov   #128,%[mask]\n"
        " mov   #0xc0,%[p0]\n"
        " mov   #0xf0,%[p1]\n"
        " mov.b @%[src],%[work]\n"
        "1:\n"
        " bit.b %[mask],%[work]\n"
        " jnz 2f\n"
        " mov.b %[p0],@%[dst]\n"
        " jmp 3f\n"
        "2:\n"
        " mov.b %[p1],@%[dst]\n"
        "3:\n"
        " inc %[dst]\n"
        " rra %[mask]\n"
        " jnz 1b\n"

        : [mask] "=&r" (mask), [work] "=&r" (work), [dst] "+&r" (dst), [p0] "=&r" (p0), [p1] "=&r" (p1)
        : [src] "r" (src)
        : "cc"
        );
#else
    unsigned register mask = 0x80;
    do {
        *dst++ = (*src & mask) ? 0b11110000 : 0b11000000;
        mask = mask >> 1;
    } while (mask);
#endif
}

#if defined(USE_DMA_SPI)
/*
 * sendRGB() send green red blue using byte_count via DMA driven SPI
 *
 * led_data - 3 bytes per led in GRB order
 */
void sendGRB(const uint8_t * const color_data, unsigned byte_cnt)
{
    // convert each bit into a 8 bit pulse for SPI
    unsigned x = 0;
    do {
        ledbits2pulse(&color_data[x], &pulse_data[x * 8]);
        x++;
    } while (x < byte_cnt);

    byte_cnt <<= 3; // use pulse_data length = byte_cnt * 8

    DMACTL0 = DMA0TSEL_19;
    __data16_write_addr((unsigned short) &DMA0SA, (unsigned long ) &pulse_data[1]); // src
    __data16_write_addr((unsigned short) &DMA0DA, (unsigned long ) &UCB0TXBUF);    // dest
    DMA0SZ = byte_cnt - 1;                        // block size
    DMA0CTL = DMADT_0 | DMASRCINCR_3 | DMASBDB; // single transfers, inc src, enable
    DMA0CTL |= DMAEN;
    UCB0TXBUF = pulse_data[0];                    // prime the SPI pump to trigger
    while (DMA0CTL & DMAEN)
        ; // wait for DMA to finish
}
#else
/*
 * sendRGB() send green red blue using byte_count
 *
 * led_data - 3 bytes per led in GRB order
 */
void sendGRB(const uint8_t * const color_data, unsigned byte_cnt)
{
    unsigned x = 0;
    do {
        ledbits2pulse(&color_data[x], &pulse_data[x * 8]);
        x++;
    } while (x < byte_cnt);


    byte_cnt <<= 3; // use pulse_data length = byte_cnt * 8

#if defined(USE_ASM_VERSION)
    register unsigned pulse_bits;

    asm (
        "1:\n"
        " mov.b @%[pulse_data]+, %[pulsebits]\n" // do one byte at a time
        "2:\n"
        " and.b %[txempty],%[txifg]\n" // spin wait for txbuf to empty
        " jz  2b\n"
        " mov.b %[pulsebits], %[txbuf]\n"// send bits
        " dec %[byte_cnt]\n"
        " jnz 1b\n"// if more, continue with next color
        :
        [byte_cnt] "+&r" (byte_cnt) /* read/write byte count */
        ,[pulsebits] "=&r" (pulse_bits) /* work register */
        :
        [pulse_data] "r" (pulse_data) /* read access to rgb data */
        ,[txbuf] "m" (UCB0TXBUF) /* address of tx buffer */
        ,[txifg] "m" (UCB0IFG) /* address of tx status flag */
        ,[txempty] "i" (UCTXIFG) /* constant bitmask for tx buffer empty*/
        : "cc"
    );
#else
    const uint8_t * dst = pulse_data;
    do {
        UCB0TXBUF = *dst++;
        while(!(UCB0IFG & UCTXIFG))
            ;

    } while(--byte_cnt);
#endif
}
#endif

void setvcoreup(unsigned int level)
{
    // Open PMM registers for write
    PMMCTL0_H = PMMPW_H;
    // Set SVS/SVM high side new level
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
    // Set SVM low side to new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
    // Wait till SVM is settled
    while ((PMMIFG & SVSMLDLYIFG) == 0)
        ;
    // Clear already set flags
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
    // Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;
    // Wait till new level reached
    if ((PMMIFG & SVMLIFG))
        while ((PMMIFG & SVMLVLRIFG) == 0)
            ;
    // Set SVS/SVM low side to new level
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
    // Lock PMM registers for write access
    PMMCTL0_H = 0x00;
}
