#include <msp430.h> 
#include "ws218x_driver.h"
#include <stdint.h>

static const uint8_t leds[BYTE_PER_LED * LED_CNT * ANIMATION_FRAMES] = {
    // 1st frame
    RGB(0x0f, 0x00, 0x00), // red   First LED
    RGB(0x00, 0x0f, 0x00), // green Second LED
    RGB(0x00, 0x00, 0x0f), // blue  Third LED
    RGB(0x2f, 0x2f, 0x2f), // white Fourth LED
    RGB(0x2f, 0x2f, 0x00), //Yellow fifth LED
    // 2nd frame
    RGB(0x0f, 0x0f, 0x0f), // w
    RGB(0x2f, 0x2f, 0x00), //Yellow fifth LED
    RGB(0x0f, 0x00, 0x00), // r
    RGB(0x00, 0x0f, 0x00), // g
    RGB(0x00, 0x00, 0x0f), // b

    // 3rd frame
    RGB(0x00, 0x00, 0x0f), // b
    RGB(0x0f, 0x0f, 0x0f), // w
    RGB(0x2f, 0x2f, 0x00), //Yellow fifth LED
    RGB(0x0f, 0x00, 0x00), // r
    RGB(0x00, 0x0f, 0x00), // g

    // 4th frame
    RGB(0x00, 0x0f, 0x00), // g
    RGB(0x00, 0x00, 0x0f), // b
    RGB(0x0f, 0x0f, 0x0f), // w
    RGB(0x2f, 0x2f, 0x00), //Yellow fifth LED
    RGB(0x0f, 0x00, 0x00), // r
};

//void gradualFill(u_int n, u_char r, u_char g, u_char b);


int main(void) {
    WDTCTL = WDTHOLD | WDTPW;

        setup();

        while (1) {
            unsigned frame_offset;
/*
            // loop through each "animation frame" 3 bytes per led, 4 leds for a total of 12 bytes
            for (frame_offset = 0; frame_offset < BYTE_PER_LED * LED_CNT * ANIMATION_FRAMES;
                frame_offset += (BYTE_PER_LED * LED_CNT)) {
                sendGRB(&leds[frame_offset], BYTE_PER_LED * LED_CNT);
                __delay_cycles(500 * (F_MCLK / 1000));
            }*/
            sendGRB(&leds[0],BYTE_PER_LED * LED_CNT);
            __delay_cycles(500 * (F_MCLK / 1000));
        }
}
/*
void gradualFill(u_int n, u_char r, u_char g, u_char b){
    int i;
    for (i = 0; i < n; i++){        // n is number of LEDs
        setLEDColor(i, r, g, b);
        showStrip();
        _delay_cycles(1000000);       // lazy delay
    }
}*/
