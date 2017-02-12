#include <msp430.h> 
#include "ws218x_driver.h"
#include <stdint.h>
#include "button.h"


static const uint8_t leds[BYTE_PER_LED * ANIMATION_LED_CNT * ANIMATION_FRAMES] = {
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

static const uint8_t shooter_leds[BYTE_PER_LED * SHOOTER_LED_CNT * SHOOTER_FRAMES] = {
    // Red frame
   RGB(0x2f, 0x00, 0x00),
   RGB(0x2f, 0x00, 0x00),
   RGB(0x2f, 0x00, 0x00),
   RGB(0x2f, 0x00, 0x00),
   RGB(0x2f, 0x00, 0x00),

   // Green frame
   RGB(0x00, 0x2f, 0x00),
   RGB(0x00, 0x2f, 0x00),
   RGB(0x00, 0x2f, 0x00),
   RGB(0x00, 0x2f, 0x00),
   RGB(0x00, 0x2f, 0x00),


};
int pressed;
int *p_pressed = &pressed;
//void gradualFill(u_int n, u_char r, u_char g, u_char b);


int main(void) {
    WDTCTL = WDTHOLD | WDTPW;
        unsigned frame_offset = 0;


        //setup the Clock and LEDs
        setup();
        /////////////

        P1DIR |= (LED0 + LED1); // Set P1.0 to output direction
        // P1.3 must stay at input
        P1OUT &= ~(LED0 + LED1); // set P1.0 to 0 (LED OFF)

        P2DIR = 0x00;  //set P2.1 to input (set it to 0);
        P2REN = 0xFF;  //turn on pull down res
        P2OUT = 0x02;
        P2IES = 0x02;
        P2IE |= BUTTON; // P2.1 interrupt enabled

        P2IFG &= ~BUTTON; // P2.1 IFG cleared



        //////////////////////////////
        sendGRB(&shooter_leds[frame_offset], BYTE_PER_LED * SHOOTER_LED_CNT);
        __enable_interrupt(); // enable all interrupts
        while (1) {

            if (pressed == 1)
                frame_offset = BYTE_PER_LED * SHOOTER_LED_CNT;
            else
                frame_offset = 0;

            /*
            // loop through each "animation frame" 3 bytes per led, 4 leds for a total of 12 bytes
            for (frame_offset = 0; frame_offset < BYTE_PER_LED * LED_CNT * ANIMATION_FRAMES;
                frame_offset += (BYTE_PER_LED * LED_CNT)) {
                sendGRB(&leds[frame_offset], BYTE_PER_LED * LED_CNT);
                __delay_cycles(500 * (F_MCLK / 1000));
            }*/
            sendGRB(&shooter_leds[frame_offset],BYTE_PER_LED * SHOOTER_LED_CNT);
            __delay_cycles(5 * (F_MCLK / 1000));
        }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
P1OUT ^= (LED0 + LED1); // P1.0 = toggle

if (*p_pressed == 0)
        *p_pressed = 1;
else
    *p_pressed = 0;

P2IFG &= ~BUTTON; // P1.3 IFG cleared
P2IES ^= BUTTON; // toggle the interrupt edge,
// the interrupt vector will be called
// when P1.3 goes from HitoLow as well as
// LowtoHigh
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
