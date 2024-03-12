uint32_t* alu(uint32_t* node, uint32_t* thrData) {
  uint32_t* ret;


typedef uint64_t  (*deadlineFuncPtr) ( uint32_t*, uint32_t* );
typedef uint32_t* (*nodeFuncPtr)  ( uint32_t*, uint32_t* );
typedef uint32_t* (*actionFuncPtr)( uint32_t*, uint32_t*, uint32_t* );
extern deadlineFuncPtr deadlineFuncs[_NODE_TYPE_END_];  ///< Function pointer array to deadline generating Functions
extern nodeFuncPtr         nodeFuncs[_NODE_TYPE_END_];  ///< Function pointer array to node handler functions
extern actionFuncPtr      actionFuncs[_ACT_TYPE_END_];  ///< Function pointer array to command action handler functions


uint8_t      roIm08(void* in);
uint16_t     roIm16(void* in);
uint32_t     roIm32(void* in);
uint64_t     roIm64(void* in);
uint8_t     roPtr08(void* in);
uint16_t    roPtr16(void* in);
uint32_t    roPtr32(void* in);
uint64_t    roPtr64(void* in);
uint8_t  roPtrPtr08(void* in);
uint16_t roPtrPtr16(void* in);
uint32_t roPtrPtr32(void* in);
uint64_t roPtrPtr64(void* in);
uint32_t     getAdr(void* in);
uint32_t     getAdr(void* in);
uint32_t     getAdr(void* in);
uint32_t     getAdr(void* in);


uint32_t   getAdr(myUnion in, myType t, myWidth w);

uint32_t* alu(uint32_t* node, uint32_t* thrData) {
  uint32_t* ret;

  uint64_t a, b, c, d, x, y, z;
  uint64_t* out;


  0 //return I0
  1 //return I1

  2 //add
  3 //sub
  4 //mul
  5 //div
  6 //mod

  7 //shl   shift X left by Y bits
  8 //shr   shift X right by Y bits
  9 //sra   shift X right by Y bits observing sign
  10 //and  X AND Y
  11 //or   X OR Y
  12 //xor  X XOR Y
  
  13 //lt
  14 //le
  15 //eq
  16 //ge
  17 //gt
  20 //mkl   mask from left
  21 //mkl   mask from right
  
  enum()

  //misc
  opc64Funcs[OPC_UNKNOWN] = dummyNodeFunc;
  opc64Funcs[OPC_OP0]     = dummyNodeFunc;
  opc64Funcs[OPC_OP1]     = tmsg;
 
  //arithm  
  opc64Funcs[OPC_ADD]     = cmd;
  opc64Funcs[OPC_SUB]     = cmd;
  opc64Funcs[OPC_MUL]     = cmd;
  opc64Funcs[OPC_DIV]     = cmd;
  opc64Funcs[OPC_MOD]     = cswitch;  
  
  //logic  
  opc64Funcs[OPC_SHL]     = cmd;
  opc64Funcs[OPC_SHR]     = cmd;
  opc64Funcs[OPC_SHA]     = cmd;
  opc64Funcs[OPC_AND]     = cmd;
  opc64Funcs[OPC_OR]      = cmd;
  opc64Funcs[OPC_XOR]     = cmd;

  //comp
  opc64Funcs[OPC_CLT]     = dummyNodeFunc;
  opc64Funcs[OPC_CLE]     = tmsg;
  opc64Funcs[OPC_CEQ]     = cmd;
  opc64Funcs[OPC_CGE]     = cmd;
  opc64Funcs[OPC_CGT]     = cswitch;  
  
  //mask
  opc64Funcs[OPC_MOL]     = cmd;
  opc64Funcs[OPC_MOR]     = cmd;





  void* op(uint8_t code, uint64_t in0, uint64_t in1, void* ret, uint8_t retType) {
    uint64_t res = opcFuncs[code](in0, in1);

    switch(type) {
      case  8 :  *(uint8_t*)ret =  (uint8_t)res; break;
      case 16 : *(uint16_t*)ret = (uint16_t)res; break; 
      case 32 : *(uint32_t*)ret = (uint32_t)res; break; 
      default : *(uint64_t*)ret = (uint64_t)res; break; 
    }

    return ret;
  }


