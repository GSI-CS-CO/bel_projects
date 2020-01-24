#include "FreeRTOS.h"
#include "lm32Interrupts.h"

/******************************************************************************
 * Local data structures                                                      *
 ******************************************************************************/

/* ISREntry table */
static ISREntry_t ISREntryTable[MAX_MICO32_ISR_LEVEL+1] = {{NULL, NULL}};


/******************************************************************************
 * Interrupt Handler (invoked by low-level routine)                           *
 * Services interrupts based on priority                                      *
 ******************************************************************************/
void ISRHandler( void )
{
    /*
     * If an interrupt-handler exists for the relevant interrupt (as detected
     * from ip and im cpu registers), then invoke the handler else disable the
     * interrupt in the im.
     */
    uint32_t ip, im, Mask, intNum;
    asm volatile ("rcsr %0,im":"=r"(im));

    do
    {
        /* read ip and calculate effective ip */
        asm volatile ("rcsr %0,ip":"=r"(ip));
        ip &= im;
        Mask = 0x1;
        intNum = 0;

        if( ip!=0 )
        {
            do
            {
                if( Mask & ip )
                {
                    if(ISREntryTable[intNum].Callback != NULL)
                    {
                        (ISREntryTable[intNum].Callback)(intNum, ISREntryTable[intNum].pContext);
                        asm volatile ("wcsr ip, %0"::"r"(Mask));
                        break;
                    }
                    else
                    {
                        asm volatile ("rcsr %0,im":"=r"(im));
                        im &= ~Mask;
                        asm volatile ("wcsr im, %0"::"r"(im));
                        asm volatile ("wcsr ip, %0"::"r"(Mask));
                        break;
                    }
                }

                Mask <<= 0x1;
                intNum++;
            }
            while(1);
        }
        else
        {
            break;
        }
    }
    while(1);
}



/******************************************************************************
 * Registers ISR callback-routine                                             *
 * Only a single source (of 32) can be registered for.                        *
 * Max intNum is determined by MAX_ISR_LEVEL and its value goes from 0->n-1   *
 ******************************************************************************/
unsigned int registerISR( const unsigned int intNum, void* pContext, ISRCallback Callback )
{
    if( intNum > MAX_MICO32_ISR_LEVEL )
    {
        return pdFAIL;
    }

    /* can be optimized by lookup table when not using barrel-shifter */
    const uint32_t mask = 1 << intNum;

    ISREntryTable[intNum].Callback = Callback;
    ISREntryTable[intNum].pContext = pContext;

    /* mask/unmask bit in the im */
    uint32_t im;
    asm volatile ("rcsr %0, im":"=r"(im));
    im = (Callback == NULL)? (im & ~mask) : (im | mask);
    asm volatile ("wcsr im, %0"::"r"(im));

      /* all done */
    return pdPASS;
}

/******************************************************************************
 * Disables a specific interrupt                                              *
 *----------------------------------------------------------------------------*
 * Inputs:                                                                    *
 *     unsigned int intNum: Interrupt-level                                   *
 * Outputs:                                                                   *
 * Return values:                                                             *
 *            MICO_STATUS_E_INVALID_PARAM                                     *
 *            MICO_STATUS_OK                                                  *
 ******************************************************************************/
unsigned int disableInterrupt( const unsigned int intNum )
{
    unsigned int im;
    unsigned int Mask;

    if(intNum > MAX_MICO32_ISR_LEVEL)
    {
        return(pdFAIL);
    }

    /* can be optimized by lookup table when not using barrel-shifter */
    Mask = ~(0x1 << intNum);

    /* disable mask-bit in im */
    asm volatile ("rcsr %0, im":"=r"(im));
    im &= Mask;
    asm volatile ("wcsr im, %0"::"r"(im));

    /* all done */
    return(pdPASS);
}


/******************************************************************************
 * Enables a specific interrupt                                               *
 * ---------------------------------------------------------------------------*
 * Inputs:                                                                    *
 *     unsigned int intNum: Interrupt-level                                   *
 * Outputs:                                                                   *
 * Return values:                                                             *
 *            MICO_STATUS_E_INVALID_PARAM                                     *
 *            MICO_STATUS_OK                                                  *
 ******************************************************************************/
unsigned int enableInterrupt( const unsigned int intNum )
{
    unsigned int im;
    unsigned int Mask;

    if( intNum > MAX_MICO32_ISR_LEVEL )
    {
        return(pdFAIL);
    }

    /* can be optimized by lookup table when not using barrel-shifter */
    Mask = (0x1 << intNum);

    /* enable mask-bit in im */
    asm volatile ("rcsr %0, im":"=r"(im));
    im |= Mask;
    asm volatile ("wcsr im, %0"::"r"(im));

    /* all done */
    return(pdPASS);
}

