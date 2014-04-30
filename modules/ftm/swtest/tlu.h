  --wb registers
   constant c_STAT      : natural := 0;               --0x00, ro, fifo n..0 status (0 empty, 1 ne)
   constant c_CLR       : natural := c_STAT     +4;   --0x04, wo, Clear channels n..0
   constant c_TEST      : natural := c_CLR      +4;   --0x08, ro, trigger n..0 status
   constant c_ACT_GET   : natural := c_TEST     +4;   --0x08, ro, trigger n..0 status
   constant c_ACT_SET   : natural := c_ACT_GET  +4;   --0x10, wo, Activate trigger n..0
   constant c_ACT_CLR   : natural := c_ACT_SET  +4;   --0x14, wo, deactivate trigger n..0
   constant c_EDG_GET   : natural := c_ACT_CLR  +4;   --0x18, ro, trigger n..0 latch edge (1 pos, 0 neg)
   constant c_EDG_POS   : natural := c_EDG_GET  +4;   --0x1C, wo, latch trigger n..0 pos
   constant c_EDG_NEG   : natural := c_EDG_POS  +4;   --0x20, wo, latch trigger n..0 neg
   constant c_IE        : natural := c_EDG_NEG  +4;   --0x24, rw, Global IRQ enable
   constant c_MSK_GET   : natural := c_IE       +4;   --0x28, ro, IRQ channels mask
   constant c_MSK_SET   : natural := c_MSK_GET  +4;   --0x2C, wo, set IRQ channels mask n..0
   constant c_MSK_CLR   : natural := c_MSK_SET  +4;   --0x30, wo, clr IRQ channels mask n..0
   constant c_CH_NUM    : natural := c_MSK_CLR  +4;   --0x34, ro, number channels present              
   constant c_CH_DEPTH  : natural := c_CH_NUM   +4;   --0x38, ro, channels depth
   --reserved
   constant c_TC_HI     : natural := 16#50#;          --0x50, ro, Current time (Cycle Count) Hi. read Hi, then Lo 
   constant c_TC_LO     : natural := c_TC_HI    +4;   --0x54, ro, Current time (Cycle Count) Lo
   constant c_CH_SEL    : natural := c_TC_LO    +4;   --0x58, rw, channels select  
-- ***** CAREFUL! From here on, all addresses depend on channels select Reg !
   constant c_TS_POP    : natural := c_CH_SEL   +4;   --0x5C, wo  writing anything here will pop selected channel
   constant c_TS_TEST   : natural := c_TS_POP   +4;   --0x60, wo, Test channels n..0
   constant c_TS_CNT    : natural := c_TS_TEST  +4;   --0x64, ro, fifo fill count
   constant c_TS_HI     : natural := c_TS_CNT   +4;   --0x68, ro, fifo q - Cycle Count Hi
   constant c_TS_LO     : natural := c_TS_HI    +4;   --0x68, ro, fifo q - Cycle Count Lo  
   constant c_TS_SUB    : natural := c_TS_LO    +4;   --0x6C, ro, fifo q - Sub cycle word
   constant c_TS_MSG    : natural := c_TS_SUB   +4;   --0x70, rw, MSI msg to be sent 
   constant c_TS_DST_ADR: natural := c_TS_MSG   +4;   --0x74, rw, MSI adr to send to
