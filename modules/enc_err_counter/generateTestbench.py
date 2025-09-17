#!/usr/bin/env python3

import os
import random
import shutil

__MODULENAME__ = "enc_err_counter"
__NERRORS__ = 100 #100
__MAX_IDLE_CYCLES__ = 100 #100
__MAX_ERROR_LENGTH__ = 10 #10
__MAX_TEST_LENGTH__ = __NERRORS__ * (__MAX_ERROR_LENGTH__ + __MAX_IDLE_CYCLES__) 

__REF_CLK_SIGNAL_NAME__ = "s_clk_ref"
__SYS_CLK_SIGNAL_NAME__ = "s_clk_sys"

#generate random clock cycle lengths with plus/minus 0.2 ns  
clk_ref = round(8 + (random.random() * 0.4) - 0.2, 2)
clk_sys = round(16 + (random.random() * 0.4) - 0.2, 2)

def make_unique(path): #implement if needed
    
    return path

def generate_clock(fout, clk_cycle, clk_signal_name):
    fout.write(f"-- Clock generator\np_{clk_signal_name} : process\nbegin\n{clk_signal_name} <= '1';\nwait for {clk_cycle/2} ns;\n{clk_signal_name} <= '0';\nwait for {clk_cycle/2} ns;\nend process;\n\n")
    return

def generate_error(fout, clk_signal_name, en_aux):
    error_length = round(random.random() * __MAX_ERROR_LENGTH__)
    aux_suffix = ""
    # if en_aux:
    #     aux_suffix = "_aux"

    fout.write(f"s_err_error <= '1';\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write(f"s_err_error <= '0';\n")
    for i in range(error_length-1):
        fout.write(f"wait until rising_edge({clk_signal_name});\n")
    return error_length

def generate_idle(fout, clk_signal_name):
    idle_cycles = round(random.random() * __MAX_IDLE_CYCLES__)
    
    fout.write(f"s_err_idle <= '1';\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write(f"s_err_idle <= '0';\n")
    for i in range(idle_cycles-1):
        fout.write(f"wait until rising_edge({clk_signal_name});\n")
    return idle_cycles

def generate_reset(fout, clk_signal_name):
    fout.write(f"s_ass_reset <= '1';\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write(f"s_ass_reset <= '0';\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")

def test_wb_output(fout,clk_signal_name, en_aux):
    fout.write(f"s_ass_set_wb <= '1';\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write(f"s_ass_set_wb <= '0';\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write(f"wait until rising_edge({clk_signal_name});\n")

def generate_error_fsm_controller(fout, nerrors, en_aux):
    #write FSM header
    fout.write(f"--Encoding Error FSM controller\np_error_fsm_controller : process\nbegin\nwait until rising_edge(s_rst_n);\n")
    fout.write("wait until rising_edge(s_clk_ref);\n")

    #generate errors
    for i in range(nerrors):
        generate_idle(fout, __REF_CLK_SIGNAL_NAME__)
        generate_error(fout, __REF_CLK_SIGNAL_NAME__, en_aux)

    fout.write("s_err_await <= '1';\n")
    fout.write("wait until rising_edge(s_clk_ref);\n")
    fout.write("s_err_await <= '0';\n")

    #wait for reset
    fout.write("wait until rising_edge(s_done_checking_error);\n")



    #generate errors
    for i in range(nerrors):
        generate_idle(fout, __REF_CLK_SIGNAL_NAME__)
        generate_error(fout, __REF_CLK_SIGNAL_NAME__, en_aux)

    fout.write("s_err_await <= '1';\n")
    fout.write("wait until rising_edge(s_clk_ref);\n")
    fout.write("s_err_await <= '0';\n")

    #process footer
    fout.write(f'end process;\n')

def generate_assertion_fsm_controller(fout, nerrors, en_aux):
     #write FSM header
    fout.write(f"--Encoding Error FSM controller\np_assertion_fsm_controller : process\nbegin\nwait until rising_edge(s_rst_n);\n")
    fout.write("wait until rising_edge(s_clk_sys);\n")

    fout.write("wait until rising_edge(s_done_generating);\n")

    fout.write("wait until rising_edge(s_clk_sys);\n")
    fout.write("wait until rising_edge(s_clk_sys);\n")
    fout.write("wait until rising_edge(s_clk_sys);\n")
    fout.write("wait until rising_edge(s_clk_sys);\n")

    test_wb_output(fout, __SYS_CLK_SIGNAL_NAME__, False)

    generate_reset(fout, __SYS_CLK_SIGNAL_NAME__)

    fout.write("wait until rising_edge(s_done_generating);\n")

    fout.write("wait until rising_edge(s_clk_sys);\n")
    fout.write("wait until rising_edge(s_clk_sys);\n")
    fout.write("wait until rising_edge(s_clk_sys);\n")
    fout.write("wait until rising_edge(s_clk_sys);\n")

    test_wb_output(fout, __SYS_CLK_SIGNAL_NAME__, False)

    #process footer
    fout.write(f'end process;\n')


    

def write_testbench(tb_name, nerrors):
    fout = open(tb_name, 'w') #add number of errors to the filename?
    #read template
    fin = open("testbenchTemplate.vhd", 'r')
    template = fin.read()
    fin.close()
    
    fout.write(template)

    #generate clocks
    generate_clock(fout, clk_ref, __REF_CLK_SIGNAL_NAME__)
    generate_clock(fout, clk_sys, __SYS_CLK_SIGNAL_NAME__)

    generate_error_fsm_controller(fout, nerrors, False)

    generate_assertion_fsm_controller(fout, nerrors, False)
    
    #write file footer
    fout.write("end architecture enc_err_counter_tb_arc;\n")
    fout.close()

    if os.path.isfile("enc_err_counter_tb.vhd"):
      os.remove("enc_err_counter_tb.vhd")
  
    shutil.copyfile(tb_name, "enc_err_counter_tb.vhd")

tb_name = f"{__MODULENAME__}_tb_ref_clk_{clk_ref}_sys_clk_{clk_sys}.vhd"
write_testbench(tb_name, __NERRORS__)