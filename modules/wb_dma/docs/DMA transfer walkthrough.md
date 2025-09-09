# DMA transfer walkthrough

address: function

0x0: transfer size

0x4: read base address

0x8: write base address

0xC: enable. This is going through an edge detection, so it has to be toggled.

Until now there is no working descriptor loading so a transfer has to be started manually. The registers above are used to configure the transfer.