/*!
 * @file scu_msi.h
 * @brief Message-Signaled Interrupts (MSI)
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.01.2020
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef _SCU_MSI_H
#define _SCU_MSI_H
#include <lm32Interrupts.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IRQ_REG_RST  0x00000000
#define IRQ_REG_STAT 0x00000004
#define IRQ_REG_POP  0x00000008

#define IRQ_OFFS_QUE 0x00000020

#define IRQ_OFFS_MSG 0x00000000
#define IRQ_OFFS_ADR 0x00000004
#define IRQ_OFFS_SEL 0x00000008


typedef struct PACKED_SIZE
{
   uint32_t reset;
   uint32_t status;
   uint32_t pop;
} MSI_CONTROL_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( MSI_CONTROL_T, reset )  == IRQ_REG_RST );
STATIC_ASSERT( offsetof( MSI_CONTROL_T, status ) == IRQ_REG_STAT );
STATIC_ASSERT( offsetof( MSI_CONTROL_T, pop )    == IRQ_REG_POP );
STATIC_ASSERT( sizeof( MSI_CONTROL_T ) == 3 * sizeof( uint32_t ) );
#endif

typedef struct PACKED_SIZE
{
   uint32_t  msg;
   uint32_t  adr;
   uint32_t  sel;
} MSI_ITEM_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( MSI_ITEM_T, msg ) == IRQ_OFFS_MSG );
STATIC_ASSERT( offsetof( MSI_ITEM_T, adr ) == IRQ_OFFS_ADR );
STATIC_ASSERT( offsetof( MSI_ITEM_T, sel ) == IRQ_OFFS_SEL );
STATIC_ASSERT( sizeof( MSI_ITEM_T ) == 3 * sizeof( uint32_t ) );
#endif

typedef struct PACKED_SIZE
{
   volatile MSI_CONTROL_T control;
   uint8_t                _RFU_[IRQ_OFFS_QUE - sizeof( MSI_CONTROL_T )];
   volatile MSI_ITEM_T    qeue[MAX_LM32_INTERRUPTS];
} MSI_IRQ_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( MSI_IRQ_T, control ) == 0 );
STATIC_ASSERT( offsetof( MSI_IRQ_T, qeue ) == IRQ_OFFS_QUE );
STATIC_ASSERT( sizeof( MSI_IRQ_T ) == IRQ_OFFS_QUE + MAX_LM32_INTERRUPTS * sizeof( MSI_ITEM_T ) );
#endif

#define MSI_IRQ_ITEM_ACCESS( M, i ) \
   __WB_ACCESS( MSI_IRQ_T, uint32_t, pCpuIrqSlave, qeue[i].M )

#define MSI_IRQ_CONTROL_ACCESS( M ) \
   __WB_ACCESS( MSI_IRQ_T, uint32_t, pCpuIrqSlave, control.M )


int msiGetBoxCpuSlot( int32_t cpuIdx, uint32_t myOffs );

int msiGetBoxSlot( uint32_t myOffs );

static inline void msiPop( MSI_ITEM_T* pItem, const unsigned int intNum )
{
   pItem->msg = MSI_IRQ_ITEM_ACCESS( msg, intNum );
   pItem->adr = MSI_IRQ_ITEM_ACCESS( adr, intNum );
   pItem->sel = MSI_IRQ_ITEM_ACCESS( sel, intNum );

   MSI_IRQ_CONTROL_ACCESS( pop ) = _irqGetPendingMask( intNum );
}


#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_MSI_H */
 /*================================= EOF ====================================*/
