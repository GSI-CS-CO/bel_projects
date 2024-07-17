import os
import random

__MODULENAME__ = "enc_err_counter"
__NERRORS__ = 100
__MAX_IDLE_CYCLES__ = 100
__MAX_ERROR_LENGTH__ = 10

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

def generate_error(fout, clk_signal_name):
    error_length = round(random.random() * __MAX_ERROR_LENGTH__)

    fout.write("enc_err <= '1';\n")
    for i in range(__MAX_ERROR_LENGTH__):
        fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write("enc_err <= '0';\n")
    return

def generate_idle(fout, clk_signal_name):
    idle_cycles = round(random.random() * __MAX_IDLE_CYCLES__)
    for i in range(idle_cycles):
        fout.write(f"wait until rising_edge({clk_signal_name});\n")

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

    #write error header
    fout.write("--Encoding Error Generator\np_enc_err_generator : process\nbegin\nwait until rising_edge(s_rst_n);")

    #generate errors
    for i in range(nerrors):
        generate_idle(fout, __REF_CLK_SIGNAL_NAME__)
        generate_error(fout, __REF_CLK_SIGNAL_NAME__)

    #assert counter value
    generate_idle(fout, __REF_CLK_SIGNAL_NAME__)
    #wait until rising_edge sys_clk
    #read slave_o.dat if counter is correct

    #write error footer
    fout.write(f"wait for {clk_sys*10000} ns;\nend process;\n")

    #write file footer
    fout.write("end architecture enc_err_counter_tb_arc;\n")
    fout.close()

tb_name = f"{__MODULENAME__}_tb_ref_clk_{clk_ref}_sys_clk_{clk_sys}.vhd"
write_testbench(tb_name, __NERRORS__)