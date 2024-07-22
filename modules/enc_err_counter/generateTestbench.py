import os
import random
import shutil

__MODULENAME__ = "enc_err_counter"
__NERRORS__ = 100 #100
__MAX_IDLE_CYCLES__ = 100 #100
__MAX_ERROR_LENGTH__ = 10 #10

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
    if en_aux:
        aux_suffix = "_aux"

    fout.write(f"enc_err{aux_suffix} <= '1';\n")
    for i in range(error_length):
        fout.write(f"wait until rising_edge({clk_signal_name});\n")
    fout.write(f"enc_err{aux_suffix} <= '0';\n")
    return error_length

def generate_idle(fout, clk_signal_name):
    idle_cycles = round(random.random() * __MAX_IDLE_CYCLES__)
    for i in range(idle_cycles):
        fout.write(f"wait until rising_edge({clk_signal_name});\n")

def test_wb_output(fout, test_data, en_aux):
    if en_aux:
        address = "00000004"
    else:
        address = "00000000"
    
    aux_suffix = ""
    if en_aux:
        aux_suffix = "_aux"
        test_data = "1"


    fout.write(f"""s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_off, x"{address}", c_reg_all_zero);\n""")
    fout.write(f"wait until rising_edge(s_wb_slave_out.ack);\nwait until rising_edge({__SYS_CLK_SIGNAL_NAME__});\n")
    fout.write(f"s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);\n")
    fout.write(f'assert s_wb_slave_out.dat = x"{test_data}"\nreport "Data read from slave{aux_suffix} is not correct! Expected number of errors: 0x{test_data}." severity NOTE;\n')
    if not en_aux:
        fout.write("s_flag <= '1';\n")

def generate_error_controller(fout, nerrors, en_aux):
    total_errors = 0

    aux_suffix = ""
    if en_aux:
        aux_suffix = "_aux"

    #write error header
    fout.write(f"--Encoding Error Generator\np_enc_err_generator{aux_suffix} : process\nbegin\nwait until rising_edge(s_rst_n);\n")
    if not en_aux:
        fout.write("s_flag <= '0';\n")

    #generate errors
    for i in range(nerrors):
        generate_idle(fout, __REF_CLK_SIGNAL_NAME__)
        total_errors += generate_error(fout, __REF_CLK_SIGNAL_NAME__, en_aux)

    #assert counter value
    generate_idle(fout, __REF_CLK_SIGNAL_NAME__)
    generate_idle(fout, __REF_CLK_SIGNAL_NAME__)
    #wait until rising_edge sys_clk
    fout.write(f"wait until rising_edge({__SYS_CLK_SIGNAL_NAME__});\n")
    #read slave_o.dat if counter is correct
    data_string = hex(total_errors).removeprefix('0x')
    if not en_aux:
        fout.write(f'nerrors <= x"{data_string.zfill(8)}";\n')
        fout.write("s_flag <= '1';\n")
    else:
        fout.write(f'nerrors_aux <= x"{data_string.zfill(8)}";\n')

    #write error footer
    fout.write(f"wait for {clk_sys*10000} ns;\nend process;\n")

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

    generate_error_controller(fout, nerrors, False)
    generate_error_controller(fout, nerrors, True)    

    # for proper testing this should wait the maximum amount of cycles the error generators could take instead of just using the template
    fin = open("testingTemplate.vhd", 'r')
    template = fin.read()
    fin.close()
    
    fout.write(template)
    
    #write file footer
    fout.write("end architecture enc_err_counter_tb_arc;\n")
    fout.close()

    os.remove("enc_err_counter_tb.vhd")
    shutil.copyfile(tb_name, "enc_err_counter_tb.vhd")

tb_name = f"{__MODULENAME__}_tb_ref_clk_{clk_ref}_sys_clk_{clk_sys}.vhd"
write_testbench(tb_name, __NERRORS__)