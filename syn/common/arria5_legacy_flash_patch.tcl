# This logic lock is needed for the legacy flash controller (not asmi).
# Without this patch, eb-flash will fail every time.
set_global_assignment -name LL_AUTO_SIZE OFF -section_id flash
set_global_assignment -name LL_CORE_ONLY OFF -section_id flash
set_global_assignment -name LL_ENABLED ON -section_id flash
set_global_assignment -name LL_HEIGHT 1 -section_id flash
set_global_assignment -name LL_IGNORE_IO_BANK_SECURITY_CONSTRAINT OFF -section_id flash
set_global_assignment -name LL_ORIGIN X28_Y1 -section_id flash
set_global_assignment -name LL_PR_REGION OFF -section_id flash
set_global_assignment -name LL_RESERVED ON -section_id flash
set_global_assignment -name LL_ROUTING_REGION_EXPANSION_SIZE 2147483647 -section_id flash
set_global_assignment -name LL_SECURITY_ROUTING_INTERFACE OFF -section_id flash
set_global_assignment -name LL_STATE LOCKED -section_id flash
set_global_assignment -name LL_WIDTH 1 -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_data_i[0]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_data_i[1]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_data_i[2]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_data_i[3]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_ncs" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_oe[0]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_oe[1]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_oe[2]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_oe[3]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_shift_o[28]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_shift_o[29]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_shift_o[30]" -section_id flash
set_instance_assignment -name LL_MEMBER_OF flash -to "monster:main|flash_top:\\flash_a5:flash|wb_spi_flash:wb|r_shift_o[31]" -section_id flash
