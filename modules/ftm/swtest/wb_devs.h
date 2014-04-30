// Priority Queue RegisterLayout
static const struct {
   unsigned int rst;
   unsigned int force;
   unsigned int dbgSet;
   unsigned int dbgGet;
   unsigned int clear;
   unsigned int cfgGet;
   unsigned int cfgSet;
   unsigned int cfgClr;
   unsigned int dstAdr;
   unsigned int heapCnt;
   unsigned int msgCntO;
   unsigned int msgCntI;
   unsigned int tTrnHi;
   unsigned int tTrnLo;
   unsigned int tDueHi;
   unsigned int tDueLo;
   unsigned int msgMin;
   unsigned int msgMax;
   unsigned int ebmAdr;
   unsigned int cfg_ena;
   unsigned int cfg_fifo;    
   unsigned int cfg_irq;
   unsigned int cfg_autopop;
   unsigned int cfg_autoflush_time;
   unsigned int cfg_autoflush_msgs;
   unsigned int force_pop;
   unsigned int force_flush;
} r_FPQ = {    .rst     =  0x00 >> 2,
               .dbgSet  =  0x04 >> 2,
               .dbgGet  =  0x08 >> 2,
               .clear   =  0x0C >> 2,
               .cfgGet  =  0x10 >> 2,
               .cfgSet  =  0x14 >> 2,
               .cfgClr  =  0x18 >> 2,
               .dstAdr  =  0x1C >> 2,
               .heapCnt =  0x20 >> 2,
               .msgCntO =  0x24 >> 2,
               .msgCntI =  0x28 >> 2,
               .tTrnHi  =  0x2C >> 2,
               .tTrnLo  =  0x30 >> 2,
               .tDueHi  =  0x34 >> 2,
               .tDueLo  =  0x38 >> 2,
               .msgMin  =  0x3C >> 2,
               .msgMax  =  0x40 >> 2,
               .ebmAdr  =  0x44 >> 2,
               .cfg_ena             = 1<<0,
               .cfg_fifo            = 1<<1,    
               .cfg_irq             = 1<<2,
               .cfg_autopop         = 1<<3,
               .cfg_autoflush_time  = 1<<4,
               .cfg_autoflush_msgs  = 1<<5,
               .force_pop           = 1<<0,
               .force_flush         = 1<<1
};



// EtherBone Master RegisterLayout
static const struct {
   unsigned int clear;
   unsigned int flush;
   unsigned int status;
   unsigned int srcMacHi;
   unsigned int srcMacLo;
   unsigned int srcIp4;
   unsigned int srcUdpPort;
   unsigned int dstMacHi;
   unsigned int dstMacLo;
   unsigned int dstIp4;
   unsigned int dstUdpPort;
   unsigned int adrHiBits;
   unsigned int maxOps;
   unsigned int ebOpt;
   unsigned int data;
   unsigned int rd;
   unsigned int wr;
   unsigned int offsDataRd;
   unsigned int offsDataWR;
   unsigned int dataAdrMsk;
   
} r_ebm = {       .clear      =  0x00 >> 2,
                  .flush      =  0x04 >> 2,
                  .status     =  0x08 >> 2,
                  .srcMacHi   =  0x0C >> 2,
                  .srcMacLo   =  0x10 >> 2,
                  .srcIp4     =  0x14 >> 2,
                  .srcUdpPort =  0x18 >> 2,
                  .dstMacHi   =  0x1C >> 2,
                  .dstMacLo   =  0x20 >> 2,
                  .dstIp4     =  0x24 >> 2,
                  .dstUdpPort =  0x28 >> 2,
                  .adrHiBits  =  0x2C >> 2,
                  .maxOps     =  0x30 >> 2,
                  .ebOpt      =  0x34 >> 2,
                  .data       =  0x00800000  >> 2,
                  .rd         =  0x00800000  >> 2,
                  .wr         =  0x00C00000  >> 2,
                  .offsDataRd =  0x00000000  >> 2,
                  .offsDataWR =  0x00400000  >> 2,
                  .dataAdrMsk =  0x003FFFFF  >> 2
};

// Timer IRQ RegisterLayout
static const struct {
   unsigned int rst;
   unsigned int armGet;
   unsigned int armSet;
   unsigned int armClr;
   unsigned int srcGet;
   unsigned int srcSet;
   unsigned int srcClr;
   unsigned int cntModeGet;
   unsigned int cntModeSet;
   unsigned int cntModeClr;
   unsigned int cscGet;
   unsigned int cscSet;
   unsigned int cscClr;
   unsigned int cntRst;
   unsigned int timerSel;
   unsigned int timeHi;
   unsigned int timeLo;
   unsigned int msg;
   unsigned int dstAdr;
   unsigned int cscSel;
   
} r_timer = {     .rst        =  0x00 >> 2,
                  .armGet     =  0x04 >> 2,
                  .armSet     =  0x08 >> 2,
                  .armClr     =  0x0C >> 2,
                  .srcGet     =  0x10 >> 2,
                  .srcSet     =  0x14 >> 2,
                  .srcClr     =  0x18 >> 2,
                  .cntModeGet =  0x1C >> 2,
                  .cntModeSet =  0x20 >> 2,
                  .cntModeClr =  0x24 >> 2,
                  .cscGet     =  0x28 >> 2,
                  .cscSet     =  0x2C >> 2,
                  .cscClr     =  0x30 >> 2,
                  .cntRst     =  0x34 >> 2,
                  .timerSel   =  0x40 >> 2, //timer select. !!!CAUTION!!! content of all regs below depends on this
                  .timeHi     =  0x44 >> 2,
                  .timeLo     =  0x48 >> 2,
                  .msg        =  0x4C >> 2,
                  .dstAdr     =  0x50 >> 2,
                  .cscSel     =  0x54 >> 2
};


