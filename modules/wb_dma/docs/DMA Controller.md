# DMA Controller

Memory can only be addressed in byte mode because the eb tools are written with byte addressing.

[Architecture](Architecture%2015882d3378bb80b8abaffcaaaeabed42.md)

[Modules and Interfaces](Modules%20and%20Interfaces%2024582d3378bb80798d2dfd23b884a40e.md)

[DMA transfer walkthrough](DMA%20transfer%20walkthrough%2025682d3378bb801c850becc60ab8f596.md)

[example implementation](https://github.com/stffrdhrn/wb_dma/tree/master)

# possible bugs

- ack count signal in testbench not synchronized → reset pulse on clk edge not detected
- the RAM between the DMA hardware and the processor doesn’t behave the same for both ports. Only after putting the DMA hardware on port 1 it worked like expected

[xwb_dpram attributes](xwb_dpram%20attributes%205fc8b924d4fd4472b0b8f2c8634649ec.md)

# TODO

- second FIFO in the channel so the read and write master can work simultaneously
- descriptor controlled transfers
    - loading descriptors until the end of the linked list is reached.