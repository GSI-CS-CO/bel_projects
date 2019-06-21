 
#pragma once

#include <string>



namespace FeSupport {
  namespace Scu {
    namespace Etherbone {
      /*!
       * \brief device port to be used for etherbone communication.
       */
      const std::string DEVICE_PORT("dev/wbm0");
      /*!
       * \brief Vendor ID to be used for etherbone communication.
       */
      enum VendorId {
        cernId = 0xce42,
        gsiId  = 0x0651
      };
      /*!
       * \brief Device ID to be used for etherbone communication.
       */
      enum DeviceId {
        scu_bus_master = 0x9602eb6f,
        gsi_gpio_32    = 0x35aa6b95,
        oled_display   = 0x93a6f3c4,
        mil_interface  = 0x35aa6b96,   //??? hat da vielleicht das ganze piggy eine eigene ID ???
        pcie_Msi_Tgt   = 0x8a670e73,
        irq_endpoint   = 0x10050082,
        irq_control    = 0x10040084,
        wb4_blockram   = 0x66cfeb52,
        wb_ddr3ram     = 0x20150828,
        wb_ddr3ram2    = 0x20160525,
        lm32_ram_user  = 0x54111351
      };
    } // namespace Etherbone
  } // namespace Scu
} // namespace FeSupport

