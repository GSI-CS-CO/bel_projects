/******************************************************************************
 *  wrunipz-api.h
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 16-January-2019
 *
 * API for wrunipz
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 17-May-2017
 ********************************************************************************************/
#ifndef __WR_UNIPZ_API_H_
#define __WR_UNIPZ_API_H_

#define WRUNIPZ_X86_VERSION "0.0.8"

#include <wr-unipz.h>

// small helper function
uint64_t getSysTime();     

// convert status code to status text
const char* wrunipz_state_text(uint32_t code              // status code
                               );

// convert state code to state text
const char* wrunipz_status_text(uint32_t code             // state code
                                );

// kill an ongoing transaction (this might leave the system in an undefined state)
uint32_t wrunipz_transaction_kill(eb_device_t device,     // EB device
                                  eb_address_t DPcmd,     // command in DP RAM
                                  eb_address_t DPstat     // status of transaction in DP RAM
                                  );

// initialize transaction for transfer of event tables 
uint32_t wrunipz_transaction_init(eb_device_t  device,    // EB device
                                  eb_address_t DPcmd,     // command in DP RAM
                                  eb_address_t DPvacc,    // # of virt acc in DP RAM
                                  eb_address_t DPstat,    // status of transaction in DP RAM
                                  uint32_t     vAcc       // virt acc to be used in transaction
                                  );

// FW starts waiting for commit event; event table uploaded to DP RAM will become activated
uint32_t wrunipz_transaction_submit(eb_device_t  device,  // EB device
                                    eb_address_t DPcmd,   // command in DP RAM
                                    eb_address_t DPstat   // status of transaction in DP RAM
                                    );

// upload event table for one specicif PZ to DP RAM
uint32_t wrunipz_transaction_upload(eb_device_t device,   // EB device
                                    eb_address_t DPstat,  // status of transaction in DP RAM
                                    eb_address_t Dpz,     // bit field in DP RAM (indicates, which PZ is submitted)
                                    eb_address_t DPdata,  // event data in DP RAM
                                    eb_address_t DPflag,  // flag data in DP RAM
                                    uint32_t pz,          // # of UNIPZ
                                    uint32_t *dataChn0,   // data for 'Kanal 0'
                                    uint32_t nDataChn0,   // # of data for 'Kanal 0'
                                    uint32_t *dataChn1,   // data for 'Kanal 1'
                                    uint32_t nDataChn1    // # of data for 'Kanal 1'
                                    );




  






#endif
