p_test_output : process
begin
wait until rising_edge(s_flag);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_off, x"00000000", c_reg_all_zero);
wait until rising_edge(s_wb_slave_out.ack);
wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
assert s_wb_slave_out.dat = nerrors
report "Data read from slave is not correct!" severity NOTE;
report "Test finished" severity NOTE;
wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000000", x"00000001");
wait until rising_edge(s_wb_slave_out.ack);
wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);

wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_off, x"00000004", c_reg_all_zero);
wait until rising_edge(s_wb_slave_out.ack);
wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
assert s_wb_slave_out.dat = nerrors_aux
report "Data read from slave is not correct!" severity NOTE;
report "Test finished" severity NOTE;
wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000004", x"00000001");
wait until rising_edge(s_wb_slave_out.ack);
wait until rising_edge(s_clk_sys);
s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
end process;
