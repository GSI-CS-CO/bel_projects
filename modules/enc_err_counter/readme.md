# PHY Encoder Error Counter

> [!IMPORTANT]  
> In this branch the ECA and the PCI-E module for the pexarria5 have been turned off for quicker build times for testing

This module counts the pulses coming from the PHY enc_err output and transfers them to the system clock domain. There they can be read via Wishbone.   
- `0x00` Error Counter 1  
- `0x04` Error Counter 2 (auxiliary)  
- `0x08` Overflow Flag 1  
- `0x0C` Overflow Flag 2

## Manual Counter Reset
**Be sure to save or write down the error count before reset. The reset completely deletes the data**
To reset a counter without a power cycle you can write a one into the counter address. The following write resets error counter one and it's overflow flag.  
`eb-write <proto/host/port> 0xXXXXXX00/4 0x00000001`  
  
The reset is a toggle. The counter won't rise as long as it is set. So the reset register has to be set to 0 again by:  
`eb-write <proto/host/port> 0xXXXXXX00/4 0x00000000`
  
## Clock Domain Crossing  
The clock domain is crossed using grey code encoding and a sync register.
