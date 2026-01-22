/*
	Blackbox configuration file: certain data from json are presented here as `defines.
*/

// ATTENTION: *.v.template file is the one to edit. *.v is automatically generated!
// See https://docs.makotemplates.org/en/latest/syntax.html for template syntax.



#ifndef BLACKBOX_CONFIG_H
#define BLACKBOX_CONFIG_H
	
	// Defines translated from json configuration

		// Versions
#define BB_VERSION_MAJOR                  0
#define BB_VERSION_MINOR                  1

		// Bus widths
#define BB_ADDR_BUS_WIDTH                 16
#define BB_DATA_BUS_WIDTH                 16

		// Amounts of resources
#define BB_NR_DIOB_IOS                    127
#define BB_NR_VIRT_IOS                    128
#define BB_NR_BACKPLANE_IOS               16
#define BB_NR_IRQ_LINES                   16
#define BB_MAX_FRONTEND_PLUGINS           256
#define BB_MAX_PROC_PLUGINS               16
#define BB_MAX_USER_PLUGINS               256
#define BB_FRONTEND_STATUS_BITS           128

		// Base addresses
#define BB_BASE_blackbox                  0x0200
#define BB_BASE_info                      0x0200
#define BB_BASE_frontend_status           0x0210
#define BB_BASE_user_sel                  0x0220
#define BB_BASE_user_area                 0x0400

		// Clock speed
#define BB_CLOCK                          125000000
	
	// Currently used configuration
#define BB_ID_config                      0

	// Frontend plugin IDs
#define BB_ID_frontend_unknown_default    255           // Default for empty/unknown frontend - disconnect everything: Default
#define BB_ID_frontend_ocio_ocio1         4             // OCIO: digital IO - control and status, 12 Channel 35 V, 10 A ('PAAL'): OCIO1
#define BB_ID_frontend_ocio_ocio2         14            // OCIO: digital IO - control and status, 12 Channel 35 V, 10 A ('PAAL'): OCIO2
#define BB_ID_frontend_ocin_ocin1         3             // OCIN: digital IO - control and status, 12 Channel 50V, 10 A (OCEM): OCIN1
#define BB_ID_frontend_interbackplane_inlb12s1    19            // INLB12S: Interbackplane for connecting up to 12 6-channel modules: INLB12S1
	// Default frontend ID
#define BB_ID_frontend_default            255

	// Proc plugin IDs
#define BB_ID_proc_disable                0             // Block all signals
#define BB_ID_proc_pass                   1             // Forward all signals in both directions
#define BB_ID_proc_in_debounce2           2             // Debounce incoming signals within 2 clock periods. Forward outgoing signals.
#define BB_ID_proc_in_debounce4           3             // Debounce incoming signals within 4 clock periods. Forward outgoing signals.
#define BB_ID_proc_in_debounce125         4             // Debounce incoming signals  within 125 clock periods. Forward outgoing signals.
	// Default proc ID
#define BB_ID_proc_default                0

	// User plugin IDs
#define BB_ID_user_gpio                   0             // A simple PIO to read/write all signals
	// Default user ID
#define BB_ID_user_default                0

	// User plugin register maps
		// gpio
#define BB_BASE_gpio_proc_sel             0x1000	
#define BB_BASE_gpio_proc_default_selected    0x1040	
#define BB_BASE_gpio_in_reg               0x1050	
#define BB_BASE_gpio_out_reg              0x1060	
#define BB_BASE_gpio_bp_in_reg            0x1100	
#define BB_BASE_gpio_bp_out_reg           0x1104	
#define BB_BASE_gpio_bp_dir_reg           0x1108	

#endif