#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "io-func.h"


uint16_t irq_count = 0;

void enableCoreVoltage (io_level_t level)
{
    if ( level == high )
    {
        PORTH.OUTSET = PIN2_bm;
    }
    else if ( level == toggle )
    {
        PORTH.OUTTGL = PIN2_bm;  
    }
    else
    {
        PORTH.OUTCLR = PIN2_bm;  
    }
}

void enable1_8V (io_level_t level)
{
    if ( level == high )
    {
        PORTH.OUTSET = PIN1_bm;
    }
    else if ( level == toggle )
    {
        PORTH.OUTTGL = PIN1_bm;  
    }
    else
    {
        PORTH.OUTCLR = PIN1_bm;  
    }
}

void enable1_8VIO (io_level_t level)
{
    if ( level == high )
    {
        PORTH.OUTSET = PIN4_bm;
    }
    else if ( level == toggle )
    {
        PORTH.OUTTGL = PIN4_bm;  
    }
    else
    {
        PORTH.OUTCLR = PIN4_bm;  
    }
}

void enable5V (io_level_t level)
{
    if ( level == high )
    {
        PORTH.OUTSET = PIN0_bm;
    }
    else if ( level == toggle )
    {
        PORTH.OUTTGL = PIN0_bm;  
    }
    else
    {
        PORTH.OUTCLR = PIN0_bm;  
    }
}


int16_t readPGoodCore (void)
{
    return ( PORTK.IN & PIN0_bm );
}

int16_t readPGood1_8V (void)
{
    return ( PORTK.IN & PIN1_bm );
}

int16_t readPGood3_3V (void)
{
    return ( PORTK.IN & PIN2_bm );
}

int16_t readPGood5V (void)
{
    return ( PORTK.IN & PIN3_bm );
}

void ADC_init(void)
{

    ADCA.CTRLA = ADC_ENABLE_bm;
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
    ADCA.REFCTRL = ADC_REFSEL_INT1V_gc;
    ADCA.PRESCALER = ADC_PRESCALER_DIV32_gc;
    ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;    //Main Power Rail (12V)
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;
    ADCA.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;    //1.8V IO Rail
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;
    ADCA.CH2.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;    //1.8V Rail
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;
    ADCA.CH3.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;    //FPGA Core Voltage Rail (0.95V)
    ADCA.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc;
}

uint16_t read_MP_ADC(void)
{
        
        ADCA.CH0.CTRL |= ADC_CH_START_bm;

        while( !(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm) );
        ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
        
    return ADCA.CH0RES;
}

uint16_t read_V1_8IO_ADC(void)
{
        
        ADCA.CH1.CTRL |= ADC_CH_START_bm;

        while( !(ADCA.CH1.INTFLAGS & ADC_CH_CHIF_bm) );
        ADCA.CH1.INTFLAGS = ADC_CH_CHIF_bm;
        
    return ADCA.CH1RES;
}

uint16_t read_V1_8_ADC(void)
{
        
        ADCA.CH2.CTRL |= ADC_CH_START_bm;

        while( !(ADCA.CH2.INTFLAGS & ADC_CH_CHIF_bm) );
        ADCA.CH2.INTFLAGS = ADC_CH_CHIF_bm;
        
    return ADCA.CH2RES;
}

uint16_t read_CORE_ADC(void)
{
        
        ADCA.CH3.CTRL |= ADC_CH_START_bm;

        while( !(ADCA.CH3.INTFLAGS & ADC_CH_CHIF_bm) );
        ADCA.CH3.INTFLAGS = ADC_CH_CHIF_bm;
        
    return ADCA.CH3RES;
}