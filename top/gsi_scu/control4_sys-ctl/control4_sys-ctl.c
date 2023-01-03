/* 
 * File: mock-lite.c
 * Description: Toggels pin PB0 every 500ms
 * From: C-Programmierung mit AVR-GCC
 */

//#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "io-func.h"




int main (void) {

    typedef enum {PWR_IDLE = 0,PWR_UP0,PWR_UP1,PWR_UP2,PWR_OK,PWR_DOWN1,PWR_DOWN0} pwr_state_t;

    pwr_state_t state = PWR_IDLE;

 

   PORTJ.DIRSET = PIN2_bm| PIN3_bm|PIN4_bm|PIN5_bm;

    //Switch to 32MHz internal oscillator 
    OSC.CTRL |= OSC_RC32MEN_bm;                 // Enable the internal 32MHz  
    while(!(OSC.STATUS & OSC_RC32MRDY_bm));     // Wait for 32MHz oscillator to stabilize 
    CCP = CCP_IOREG_gc;                         // Disable register security for clock update
    CLK.CTRL = CLK_SCLKSEL_RC32M_gc;            // Switch to 32MHz clock
    OSC.CTRL &= ~OSC_RC2MEN_bm;                 // Disable 2Mhz oscillator

    ADC_init();

    //Output Init

    enableCoreVoltage (low);
    enable1_8V (low);
    enable1_8VIO (low);
    enable5V (low);

   while(1) {

        switch(state)
        {
            case PWR_IDLE:     
                if (MP_ADC_read() >= MP_ON_ADC_THRES)
                {
                state = PWR_UP0;
                }
            break;
            case PWR_UP0:
                enableCoreVoltage (high);
                if (readPGoodCore())
                {
                state = PWR_UP1;
                }
            break;
            case PWR_UP1:
                enable1_8V (high);
                if (readPGood1_8V())
                {
                state = PWR_UP2;
                }
            break;
            case PWR_UP2:
                enable1_8VIO (high);
                if (readPGood1_8VIO())
                {
                state = PWR_OK;
                }
            break;
            //Power Down
            case PWR_DOWN1:
                enable5V (low);
                enable1_8VIO (low);
                enable1_8V (low);   
                state = PWR_DOWN0;
            break;
            case PWR_DOWN0:
                enableCoreVoltage (low);
                state = PWR_IDLE;
            break;
            // Power OK
            case PWR_OK:
                if (MP_ADC_read() <= MP_FAIL_ADC_THRES)           
                {
                state = PWR_DOWN1;
                }
                enable5V (high);
            break;
         }
        
   }

   return 0;
}