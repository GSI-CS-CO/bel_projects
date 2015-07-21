create_clock -period 12500KHz -name SCUBUSCLK   [get_ports {A_SYSCLOCK}]
create_clock -period 20MHz    -name CLK_20MHZ_D [get_ports {CLK_20MHZ_D}]
create_clock -period 12500KHz -name {SCU_Bus_Slave:SCU_Slave|S_Powerup_Res} {SCU_Bus_Slave:SCU_Slave|S_Powerup_Res}

    #create_generated_clock -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -multiply_by 10 -duty_cycle 50.00 -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]} {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}
    #create_generated_clock -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[1]} -multiply_by 20 -duty_cycle 50.00 -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]~1} {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]}
    #create_generated_clock -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -multiply_by 20 -duty_cycle 50.00 -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]} {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]}
    #create_generated_clock -source {diob_clk_switch|local_clk|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 8 -multiply_by 5 -duty_cycle 50.00 -name {diob_clk_switch|local_clk|altpll_component|auto_generated|pll1|clk[0]} {diob_clk_switch|local_clk|altpll_component|auto_generated|pll1|clk[0]}



derive_pll_clocks 
derive_clock_uncertainty

create_clock -period 40MHz -name altera_reserved_tck altera_reserved_tck

set_clock_groups -asynchronous                           \
 -group { altera_reserved_tck                          } \
 -group { CLK_20MHZ_D                                  } \
 -group { SCUBUSCLK                                    }
 
# -group { [get_ports {altera_reserved_tck}]           } \
# -group { [get_ports {CLK_20MHZ_D}]                   } \
# -group { [get_ports {SCUBUSCLK}]                     } 


