module altera_wrapper_debounce (
  clk,
  reset_n,
  data_in,
  data_out
);

  input  wire clk;
  input  wire reset_n;
  input  wire [3:0] data_in;
  output wire [3:0] data_out;

  parameter WIDTH = 4;
  parameter POLARITY = "LOW";
  parameter TIMEOUT = 50000;
  parameter TIMEOUT_WIDTH = 16;

  debounce debounce_inst (
    .clk                                  (clk),
    .reset_n                              (reset_n),
    .data_in                              (data_in),
    .data_out                             (data_out)
  );

endmodule
