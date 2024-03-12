/** @name Generic node layout definitions. Goes for all nodes */
//@{
#define NODE_BEGIN              (0)		///< First word
#define NODE_OPT_DYN            (0x24)   ///< Word definitions if we have dynamic fields
#define NODE_HASH               (0x28)	///< 32b hash of the node name string
#define NODE_FLAGS              (NODE_HASH  + _32b_SIZE_) ///< Flag field
#define NODE_DEF_DEST_PTR       (NODE_FLAGS + _32b_SIZE_) ///< Default destination (successor) pointer
//@}




/** @name Timing Message node layout definitions */
//@{
#define ALU_BEGIN               (NODE_BEGIN)				///< First word
#define ALU_IN_A                (ALU_BEGIN) 				
#define ALU_IN_B              	(ALU_IN_A     + _PTR_SIZE_)	
//#define ALU_IN_A64			(ALU_BEGIN)
#define ALU_IN_C              	(ALU_IN_B     + _PTR_SIZE_)	
#define ALU_IN_D              	(ALU_IN_C     + _PTR_SIZE_)	
//#define ALU_IN_C64            (ALU_IN_B     + _PTR_SIZE_)	

#define ALU_OUT              	(ALU_IN_D     + _PTR_SIZE_)	
//#define ALU_OUT_HI            (ALU_IN_D     + _PTR_SIZE_)	
//#define ALU_OUT_LO            (ALU_OUT_OH   + _32b_SIZE_)	
//#define ALU_OUT64             (ALU_IN_D     + _PTR_SIZE_)	
#define ALU_ALT_DST             (ALU_OUT   	  + _64b_SIZE_)	
#define ALU_OPCODES             (ALU_ALT_DS   + _PTR_SIZE_)	
#define ALU_RES             	(ALU_OPCODE   + _32b_SIZE_)	

//@}


/** @name Node flag field bit defs - Position of type specific flags */
//@{
#define NFLG_BITS_SPECIFIC_POS  20
//@}

/////////////////////////////////////////////////////////////////////
// ALU
/////////////////////////////////////////////////////////////////////

#define ALU_OUTSEL_X      0  ///< Tmsg - Address of dynamic ID source
#define ALU_OUTSEL_Y      1  ///< Tmsg - Address of dynamic ID source
#define ALU_OUTSEL_Z      2  ///< Tmsg - Address of dynamic ID source

/** @name Node flag field bit defs - ALU output select */
//@{
#define NFLG_ALU_OUTSEL_MSK    0x2
#define NFLG_ALU_OUTSEL_POS    (NFLG_BITS_SPECIFIC_POS + 0)
#define NFLG_ALU_OUTSEL_SMSK   (NFLG_ALU_OUTSEL_MSK << NFLG_ALU_OUTSEL_POS)
//@}

/** @name Node flag field bit defs - ALU computation mode 0 ((A B)(C D)) 1 ((A B)C)D) */
//@{
#define NFLG_ALU_CMODE_MSK    0x1
#define NFLG_ALU_CMODE_POS    (NFLG_BITS_SPECIFIC_POS + 2)
#define NFLG_ALU_CMODE_SMSK   (NFLG_ALU_CMODE_MSK << NFLG_ALU_CMODE_POS)
//@}

/** @name Node flag field bit defs - ALU Branch if true */
#define NFLG_ALU_BRT_MSK      0x1
#define NFLG_ALU_BRT_POS      (NFLG_BITS_SPECIFIC_POS + 3)
#define NFLG_ALU_BRT_SMSK     (NFLG_ALU_BRT_MSK << NFLG_ALU_BRT_POS)
//@}