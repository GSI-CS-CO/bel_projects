/** Copyright (C) 2018,2023 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  @author Michael Reese <m.reese@gsi.de>
 *
 *******************************************************************************
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
 *******************************************************************************
 */
 
#ifndef WR_MIL_GATEWAY_H_
#define WR_MIL_GATEWAY_H_

#include <saftlib/Owned.hpp>
#include <saftlib/OpenDevice.hpp>
#include <saftlib/TimingReceiver.hpp>
#include <saftlib/TimingReceiverAddon.hpp>

#include <saftbus/loop.hpp>

namespace saftlib {

class TimingReceiver;

class WrMilGateway : public Owned, public TimingReceiverAddon
{
	
  public:
    // @saftbus-export
    void StartSIS18();
    // @saftbus-export
    void StartESR();
    // @saftbus-export
    void ClearStatistics();
    // @saftbus-export
    void ResetGateway();
    // @saftbus-export
    void KillGateway();
    // @saftbus-export
    void UpdateOLED();
    // @saftbus-export
    void RequestFillEvent();
    // @saftbus-export
    void IncrementLateMilEvents();
    // @saftbus-export
    void ResetLateMilEvents();

    // @saftbus-export
    std::vector< uint32_t > getRegisterContent()  const;
    // @saftbus-export
    std::vector< uint32_t > getMilHistogram()     const;
    // @saftbus-export
    uint32_t                getWrMilMagic()       const;
    // @saftbus-export
    uint32_t                getFirmwareState()    const;
    // @saftbus-export
    uint32_t                getEventSource()      const;
    // @saftbus-export
    unsigned char          getUtcTrigger()       const;
    // @saftbus-export
    uint32_t                getEventLatency()     const;
    // @saftbus-export
    uint32_t                getUtcUtcDelay()      const;
    // @saftbus-export
    uint32_t                getTriggerUtcDelay()  const;
    // @saftbus-export
    uint64_t                getUtcOffset()        const;
    // @saftbus-export
    uint64_t                getNumMilEvents()     const;
    // @saftbus-export
    std::vector< uint32_t > getLateHistogram()    const;
    // @saftbus-export
    uint32_t                getNumLateMilEvents() const;
    // @saftbus-export
    bool                   getFirmwareRunning()  const;
    // @saftbus-export
    bool                   getInUse()            const;

    // @saftbus-export
    void setUtcTrigger(unsigned char val);
    // @saftbus-export
    void setEventLatency(uint32_t val);
    // @saftbus-export
    void setUtcUtcDelay(uint32_t val);
    // @saftbus-export
    void setTriggerUtcDelay(uint32_t val);
    // @saftbus-export
    void setUtcOffset(uint64_t val);
    // @saftbus-export
    void setOpReady(bool val);

    // @saftbus-export
    sigc::signal<void, uint32_t> SigFirmwareState;
    // @saftbus-export
    sigc::signal<void, bool>     SigFirmwareRunning;
    // @saftbus-export
    sigc::signal<void, uint32_t> SigEventSource;
    // @saftbus-export
    sigc::signal<void, uint32_t> SigReceivedMilEvent;
    // @saftbus-export
    sigc::signal<void, bool>     SigInUse;
    
    WrMilGateway(saftlib::SAFTd *sd, saftlib::TimingReceiver *tr, saftbus::Container *container);
    ~WrMilGateway();

    const std::string& getObjectPath() const;
    std::map< std::string, std::map< std::string, std::string> > getObjects();
  private:
    void Reset();
    void ownerQuit();

    void oledUpdate();

    // Polling method
    bool poll();
    const int poll_period; // [ms]


    uint32_t readRegisterContent(uint32_t reg_offset) const;
    void    writeRegisterContent(uint32_t reg_offset, uint32_t value);
    bool    firmwareRunning() const;

    mutable bool    firmware_running;
    mutable uint32_t firmware_state;
    mutable uint32_t event_source;
    mutable uint32_t num_late_events;
    uint64_t num_mil_events;
    const uint32_t max_time_without_mil_events; // if time_without_events exceeds this, we conclude the gateway isn't used
    struct timespec time_of_last_mil_event;


    saftbus::SourceHandle pollConnection;

    eb_address_t  oled_reset;
    eb_address_t  oled_char;
    eb_address_t  mil_events_present;
    eb_address_t  mil_event_read_and_pop;

    etherbone::Device&     device;

    struct sdb_device         wrmilgw_device; // store the LM32 device with WR-MIL-Gateway firmware running
    eb_address_t              base_addr;
    etherbone::sdb_msi_device sdb_msi_base;
    sdb_device                mailbox;
    eb_address_t              irq;
    bool                      have_wrmilgw;
    bool                      idle;

    eb_address_t              mailbox_slot_address;
    unsigned mbx_slot;

    std::string object_path;

};

}

#endif
