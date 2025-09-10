# Architecture

# Control Interface host → real time system

The host system writes a descriptor into the dual port RAM (every so and so cycles, if it otherwise stalls the RAM completely, implemented with a simple shifter scheduler).

The DMA controller then decides based on multiple inputs if a channel should be activated. This could be done with a start bit, a check if the address is correct or any other descriptor validity check (is this necessary, or would one start bit be enough? flipped bit?).

# DMA DPRAM

This RAM contains the firmware for the RISC-V soft core and the transfer descriptors for the DMA hardware logic. This RAM needs to be transparent to the host system to be able to program the firmware during runtime instead of having to program it via RAM initialization files. Also for debugging of course.

# DPRAM BUFFER:

multi producer situations need to be avoided so the bus and the bus interfaces don’t get blocked → thus the host shouldn’t directly write into the RISC-V DPRAM

DPRAM could be integrated into slave logic module?

# PSRAM interface

The host system is the system hosting the PCIe Card (Pexarria5). When it wants to create a new timing event, it will have to write it into the PSRAM/CACHE? But to avoid using an arbiter and to simplify memory access, it is written into a DPRAM buffer, from where it is copied by the memory controller. (To both PSRAM and cache/LM32Cluster memory?). The cache/memory coherency will be handled by the RISC-V memory controller with standard methods.

**THUS THE PSRAM WILL ONLY BE ACCESSIBLE BY THE MEMORY CONTROLLER, NOT DIRECTLY FROM THE HOST SYSTEM.**

# Slave Logic

The slave logic is responsible for any wishbone bus communication with either the DMA logic or the RISC-V processor.

This especially includes the configuration of the DMA logic after a reset or power up. The configuration consists of the accessible address range on the shared RAM, number of channels and so on.

# DMA Hardware

Two master interfaces, split into a read and a write interface (but both still have full capabilities, they get configured with the CSR registers).

The transfer hardware is held as simple as possible, as the firmware is supposed to take the brunt of the work load.
~~From the read master the data is run through a skid pad~~. The skidpads are handled slave side. The master has to react to STALL signals. ~~After the skid pad~~ it gets put into a FIFO to take care of bus congestion. This requires the read interface to be favored if the buffer is more empty, and the write interface interface to be favored if the buffer is almost full.
Multiple buffers are necessary if a switch between multiple channels during transfer is supposed to be possible immediately. Otherwise the DMA can empty the buffer and write the information to the a smaller “read/written/transferred” memory to make the data transparent to the host.

If all descriptors need to be finished before changing the channel it gets even easier, as there is only on buffer and one “read/written/transferred” register instead of for every channel.

Each channel only has one descriptor active. If the amount of channels is too little the firmware will have to stall and write the new descriptors as soon as a channel has cleared.

## Descriptor Loading

The descriptor gets loaded with the read master and should be saved in the channel. Right now that connection does not exist yet as the interface is wrong and the control logic is not implemented in the read master and channel. The channel should accept data when an ACK arrives and then skip to the next data word.

## Buffer Architecture

The data buffer is only implemented once and is shared by all descriptors. This means a transfer shouldn't be paused (if it doesn't get repeated?). The channels should contain the descriptor data and any benchmarking or information registers.

# “Flow Chart”

## New Data from Host

1. Host writes new data into DPRAM
2. when done tells the slave logic that data is ready and gives slave transfer request
3. RISC-V elaborates transfer request into descriptors, which it writes into the DMA DPRAM
4. RISC-V tells the slave logic to start OR DMA registers a start signal from the channel csr?
5. DMA takes care of memory transfers from DPRAM buffer to PSRAM or LM32 cluster or others?

## Only Transfer Instruction from Host

1. Host writes transfer request into slave logic
2. RISC-V elaborates transfer request into descriptors
3. DMA takes care of memory transfers