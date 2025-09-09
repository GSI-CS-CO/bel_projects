# Modules and Interfaces

# DMA

Main module containing the instances of all the submodules.

Its interface is the one going out to the monster.vhd.

- g_host_ram_size: The size of the RAM of the RISCV processor (old name. should be changed to something more appropriate)
- g_dma_transfer_block_size: maximum transfer block size for which the DMA gets synthesized
- slave: Control slave to start and configure the DMA (a lot of debugging is done by it as well)
- ram_slave: wishbone interface of the RAM so it is accessible for testing and later for firmware flashing
- masters: these are the wishbone masters for the DMA logic

# RAM

RAM module that is connected to the wishbone bus and (later) the RISCV processor.

# Engine (incomplete)

Main logic of the DMA, here the FSM for controlling the DMA should be.

- descriptor_write_select_o: select which part of the descriptor should be written, instead of having an enable signal for every word.
- init_pointer_addr_i: pointer address at which the first descriptor is saved if a new descriptor list is started.
- next_pointer_addr_i: the address at the end of the current descriptor pointing to the next one
- pointer_addr_o: address used for memory access
- communication signals: start or stop the transfer

# Channel (incomplete)

Small memory block to save the currently active descriptor for a channel.

- g_max_transfer_size: max transfer size the channel is synthesized for
- write_select: select descriptor word to write

# Read Master

Read Master that reads a block starting from the start address and of the transfer size. For reading a descriptor from the RAM it has a separate mode with its own signals. 

- g_block_size: maximum transfer block size that is synthesized
- read_descriptor_i: config signal to read descriptor or use transfer mode
- rd_buffer_ready_i: signal if the read buffer can take new data

# Write Master

Write master to write the data back to the wishbone bus/memory. It has the same signals as the read master, excluding the descriptor ones.

# Data Buffer

A FIFO to store the data block between reading and writing it over wishbone. The snoop inputs are there to be able to save/advance the data in the FIFO depending on if the masters have read/written this cycle.

- g_block_size: max transfer size the buffer is synthesized for