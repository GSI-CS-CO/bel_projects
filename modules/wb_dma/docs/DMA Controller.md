# DMA Controller

Memory can only be addressed in byte mode because the eb tools are written with byte addressing.

[Architecture](Architecture.md)

[Modules and Interfaces](Modules%20and%20Interfaces.md)

[DMA transfer walkthrough](DMA%20transfer%20walkthrough.md)

[example implementation](https://github.com/stffrdhrn/wb_dma/tree/master)

# possible bugs

- ack count signal in testbench not synchronized → reset pulse on clk edge not detected
- the RAM between the DMA hardware and the processor doesn’t behave the same for both ports. Only after putting the DMA hardware on port 1 it worked like expected

[xwb_dpram attributes](xwb_dpram%20attributes.md)

# TODO

- second FIFO in the channel so the read and write master can work simultaneously
- descriptor controlled transfers
    - loading descriptors until the end of the linked list is reached.