`ifndef BLACKBOX_DEFINES_VH
`define BLACKBOX_DEFINES_VH

// ===== Helpful macros =====

	// Get 'nr'-th 'width'-wide slice of vector 'vect'. Useful for slicing secondary data bus in flex_hub & flex_superhub 
`define slice(vect,width,nr)	vect[((nr)+1)*(width)-1 -: (width)]
	// Round-up (ceil) division for calculating registers needed for certain amounts of bits
`define ceildiv(num,den)	(((num)+(den)-1)/(den))

`endif
