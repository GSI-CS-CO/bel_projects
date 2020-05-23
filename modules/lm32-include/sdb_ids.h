/*!
 * @brief Definition of SDB addresses, outsourced from
 *        mini_sdb.h
 *
 * @file      sdb_ids.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.02.2019
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
#ifndef _SDB_IDS_H
#define _SDB_IDS_H

///////////////////////////////////////////////////////////////
//  SBD BASE ADR IS AUTOMAPPED IN GATEWARE. USE getRootSdb() //
///////////////////////////////////////////////////////////////

#define SDB_INTERCONNET 0x00
#define SDB_DEVICE      0x01
#define SDB_BRIDGE      0x02
#define SDB_MSI         0x03
#define SDB_EMPTY       0xFF



#define ERROR_NOT_FOUND  0xFFFFFFFE
#define NO_MSI           0XDEADBEE3
#define OWN_MSI          (1<<31)


#define GSI                   0x00000651
#define CERN                  0x0000ce42


//MSI message forwarding box for master2master MSI
#define MSI_MSG_BOX           0xfab0bdd8

//CPU periphery
#define CPU_INFO_ROM          0x10040085
#define CPU_ATOM_ACC          0x10040100
#define CPU_SYSTEM_TIME       0x10040084
#define CPU_TIMER_CTRL_IF     0x10040088
#define CPU_MSI_CTRL_IF       0x10040083
#define CPU_MSI_TGT           0x1f1a4e39

//Cluster periphery
#define LM32_CB_CLUSTER       0x10041000
#define CLU_INFO_ROM          0x10040086
#define LM32_RAM_SHARED       0x81111444
#define FTM_PRIOQ_CTRL        0x10040200
#define FTM_PRIOQ_DATA        0x10040201

//External interface to CPU RAMs & IRQs
#define LM32_RAM_USER         0x54111351
#define LM32_IRQ_EP           0x10050083

//Generic stuff
#define CB_GENERIC            0xeef0b198
#define DPRAM_GENERIC         0x66cfeb52
#define IRQ_ENDPOINT          0x10050082
#define PCIE_IRQ_ENDP         0x8a670e73

//IO Devices
#define OLED_DISPLAY          0x93a6f3c4
#define SSD1325_SER_DRIVER    0x55d1325d
#define ETHERBONE_MASTER      0x00000815
#define ETHERBONE_CFG         0x68202b22


#define ECA_EVENT             0x8752bf45
#define ECA_CTRL              0x8752bf44
#define TLU                   0x10051981
#define WR_UART               0xe2d13d04
#define WR_PPS_GEN            0xde0d8ced
#define SCU_BUS_MASTER        0x9602eb6f
#define SCU_IRQ_CTRL          0x9602eb70
#define WB_FG_IRQ_CTRL        0x9602eb71
#define MIL_IRQ_CTRL          0x9602eb72

#define SCU_BUS_MASTER        0x9602eb6f
#define WR_1Wire              0x779c5443
#define User_1Wire            0x4c8a0635
#define WB_FG_QUAD            0x863e07f0

#define WR_CFIPFlash          0x12122121
#define WB_DDR3_if1           0x20150828
#define WB_DDR3_if2           0x20160525
#define WR_SYS_CON            0xff07fc47
#define WB_REMOTE_UPDATE      0x38956271
#define WB_ASMI               0x48526423
#define WB_SCU_REG            0xe2d13d04

#endif /* ifndef _SDB_IDS_H */
/*================================== EOF ====================================*/
