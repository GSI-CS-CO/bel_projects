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

void enableComXpowerOk (io_level_t level)
{
    if ( level == high )
        {
            PORTF.OUTSET = PIN3_bm;
        }
    else if ( level == toggle )
        {
            PORTF.OUTTGL = PIN3_bm;
        }
    else
        {
            PORTF.OUTCLR = PIN3_bm;
        }
}

void enableIO (void)
{
    PORTA.OUTSET = PIN6_bm;
    PORTA.OUTCLR = PIN7_bm;
}

void disableIO (void)
{
    PORTA.OUTSET = PIN7_bm;
    PORTA.OUTCLR = PIN6_bm;
}

//Reset Functions

void performReset(void)
{
    nExtResetOut(low);
    nPCIeResetOut(low);
    nSysOut(low);
    nFPGAOut(low);
}

void releaseReset(void)
{
    nExtResetOut(high);
    nPCIeResetOut(high);
    nSysOut(high);
    nFPGAOut(high);
}

int16_t nExtResetOut(io_level_t level)
{
    if ( level == high )
        {
            PORTF.OUTSET = PIN4_bm;
        }
    else if ( level == toggle )
        {
            PORTF.OUTTGL = PIN4_bm;
        }
    else
        {
            PORTF.OUTCLR = PIN4_bm;
        }
}

int16_t nPCIeResetOut(io_level_t level)
{
    if ( level == high )
        {
            PORTF.OUTSET = PIN5_bm;
        }
    else if ( level == toggle )
        {
            PORTF.OUTTGL = PIN5_bm;
        }
    else
        {
            PORTF.OUTCLR = PIN5_bm;
        }
}

int16_t nSysOut(io_level_t level)
{
    if ( level == high )
        {
            PORTF.OUTSET = PIN6_bm;
        }
    else if ( level == toggle )
        {
            PORTF.OUTTGL = PIN6_bm;
        }
    else
        {
            PORTF.OUTCLR = PIN6_bm;
        }
}

int16_t nFPGAOut(io_level_t level)
{
    if ( level == high )
        {
            PORTF.OUTSET = PIN7_bm;
        }
    else if ( level == toggle )
        {
            PORTF.OUTTGL = PIN7_bm;
        }
    else
        {
            PORTF.OUTCLR = PIN7_bm;
        }
}

int16_t nWakeOut(io_level_t level)
{
    if ( level == high )
        {
            PORTB.OUTSET = PIN6_bm;
        }
    else if ( level == toggle )
        {
            PORTB.OUTTGL = PIN6_bm;
        }
    else
        {
            PORTB.OUTCLR = PIN6_bm;
        }
}

void indicatorLED(led_color_t color)
{

    int16_t redLED(io_level_t level)
    {
        if ( level == high )
            {
                PORTE.OUTSET = PIN0_bm;
            }
        else if ( level == toggle )
            {
                PORTE.OUTTGL = PIN0_bm;
            }
        else
            {
                PORTE.OUTCLR = PIN0_bm;
            }
    }

    int16_t greenLED(io_level_t level)
    {
        if ( level == high )
            {
                PORTE.OUTSET = PIN4_bm;
            }
        else if ( level == toggle )
            {
                PORTE.OUTTGL = PIN4_bm;
            }
        else
            {
                PORTE.OUTCLR = PIN4_bm;
            }
    }

    int16_t blueLED(io_level_t level)
    {
        if ( level == high )
            {
                PORTE.OUTSET = PIN3_bm;
            }
        else if ( level == toggle )
            {
                PORTE.OUTTGL = PIN3_bm;
            }
        else
            {
                PORTE.OUTCLR = PIN3_bm;
            }
    }

    switch (color)
        {
        case red:
            redLED(low);
            greenLED(high);
            blueLED(high);
            break;

        case green:
            redLED(high);
            greenLED(low);
            blueLED(high);
            break;

        case blue:
            redLED(high);
            greenLED(high);
            blueLED(low);
            break;

        case yellow:
            redLED(low);
            greenLED(low);
            blueLED(high);
            break;

        case cyan:
            redLED(high);
            greenLED(low);
            blueLED(low);
            break;

        case magenta:
            redLED(low);
            greenLED(high);
            blueLED(low);
            break;

        case white:
            redLED(low);
            greenLED(low);
            blueLED(low);
            break;

        case ledOff:
            redLED(high);
            greenLED(high);
            blueLED(high);
            break;

        default:
            redLED(high);
            greenLED(high);
            blueLED(high);
            break;

        }

}


//Read Functions


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

// ComX Watchdog
int16_t readWDT (void)
{
    return ( PORTB.IN & PIN0_bm );
}

int16_t readUserPushButton (void)
{
    return ( PORTR.IN & PIN0_bm );
}

//FPGA Status

int16_t readConfDone (void)
{
    return ( PORTD.IN & PIN3_bm );
}

int16_t readnStatus (void)
{
    return ( PORTD.IN & PIN4_bm );
}

int16_t readInitDone (void)
{
    return ( PORTD.IN & PIN6_bm );
}




//Reset read functions

int16_t readResetButton (void)
{
    return ( PORTR.IN & PIN1_bm );
}

int16_t readExtensionReset (void)
{
    return ( PORTQ.IN & PIN0_bm );
}

int16_t readBackplaneReset (void)
{
    return ( PORTQ.IN & PIN1_bm );
}

int16_t readFPGAReset (void)
{
    return ( PORTJ.IN & PIN3_bm );
}

int16_t readCOMXReset (void)
{
    return ( PORTB.IN & PIN7_bm );
}

int16_t readAllResets (void)
{
    if (readResetButton() && readFPGAReset() && readBackplaneReset() && readExtensionReset() && readCOMXReset())
        {
            return 1; // No Reset
        }
    else
        {
            return 0; //low active Reset
        }

}

//ADC Functions

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