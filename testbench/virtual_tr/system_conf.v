`ifdef SYSTEM_CONF_V
`else
`define SYSTEM_CONF_V


`define CFG_EBA_RESET    32'h00000000
`define CFG_SDB          32'h00000000

`define CFG_PL_MULTIPLY_ENABLED      1
`define CFG_PL_BARREL_SHIFT_ENABLED  1
`define CFG_SIGN_EXTEND_ENABLED      1
`define CFG_INTERRUPTS_ENABLED       1
// Instruction cache
`define CFG_ICACHE_ENABLED         1
`define CFG_ICACHE_ASSOCIATIVITY   1
`define CFG_ICACHE_SETS            512
`define CFG_ICACHE_BYTES_PER_LINE  4
`define CFG_ICACHE_BASE_ADDRESS    32'h00000000
`define CFG_ICACHE_LIMIT           32'h7fffffff

`endif