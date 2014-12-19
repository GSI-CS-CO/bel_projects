# NAU8811 Audio Driver with Wishbone Interface

---

##Contents

* Synopsis
* Register Layout

---

## Synopsis

This unit can control a NAU8811 Audio Chip. Furthermore there are two mandatory interfaces:

* SPI (for control)
* IIS/I²S/I2S (for audio data)

Seven pins are used:

* Serial Clock (SCLK)
* Serial Data (SDIO)
* Serial Chip/Slave Select (CSB)
* Audio Bit Clock (bclk)
* Audio Frame Sync (fs)
* Audio DAC Out (dacout)
* Audio ADC in (adcin)

A wishbone interface with seven registers is provided by this unit. For details see section "Register Layout".
Note that the serial data is transmitted in two's complement with the MSB first.

Signal characteristics:

* f{pll_input} = 24.576MHz (must be provided internally by the FPGA/ASIC)
* f{bit_clock} = 12.288MHz
* f{frame_sync} = 48kHz
* Word width = 32-bit (NAU8811 will use the 24 upper bits)
* FIFO size = 512*4 Bytes 

---

## Register Layout

### GENERIC CONTROL REGISTER @ 0x00
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-10  |        0x0 |  R  | -Reserved-                                                        | 
|    9   |        0x0 | R/W | HEARTBEAT_POLARITY: 0 => Falling edge - 1 => Rising edge |
|    8   |        0x0 | R/W | HEARTBEAT_MODE: 0 => No heartbeat trigger - 1 => Play sound on trigger |
|    7   |        0x0 | R/W | INTERNAL_LOOP: 0 => Don't loop back data - 1 => Loop data back |
|    6   |        0x0 | R/W | PADDING: X => Padding Value for unused bits at the audio frame |
|    5   |        0x0 | R/W | INVERT_FS: 0 => Don't invert FS - 1 => Invert FS |
|    4   |        0x0 | R/W | MONO_OUTPUT: 0 => Clone left channel to right channel - 1 => Only send data for left channel. |
|    3   |        0x0 | R/W | TRANSACTION_MODE: 0 => Only receive data when transmitting new data - 1 => Receive new data all the time. | 
|    2   |        0x0 | R/W | CLOCK_ENABLE: 0 => Disable bit clock and frame sync - 1 => Enable bit clock and frame sync | 
|    1   |        0x0 | R/W | SS_STATE: 0 => Set SS to LOW - 1 => Set SS to HIGH |  
|    0   |        0x0 | R/W | SS_CTRL_CONFIG: 0 => SPI manages SS - 1 => Set SS by hand (according to SS_STATE value) | 

### GENERIC STATUS REGISTER @ 0x04
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-6   |        0x0 |  R  | -Reserved-                                                        | 
|    5   |        0x0 |  R  | IIS RX FIFO empty.                                                | 
|    4   |        0x0 |  R  | IIS RX FIFO full.                                                 | 
|    3   |        0x0 |  R  | IIS TX FIFO empty.                                                | 
|    2   |        0x0 |  R  | IIS TX FIFO full.                                                 | 
|    1   |        0x0 |  R  | SPI TX FIFO empty.                                                | 
|    0   |        0x0 |  R  | SPI TX FIFO full.                                                 | 

### SPI TX FIFO DATA REGISTER @ 0x08
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-16  |        0x0 |  R  | -Reserved-                                                        | 
| 15-0   |        0x0 |  W  | Write: Put a two byte into the transmit FIFO.                     | 

### IIS TX FIFO DATA REGISTER @ 0x0C
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-0   |        0x0 |  W  | Write: Put a double word into the transmit FIFO.                  | 

### IIS RX FIFO DATA REGISTER @ 0x10
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-0   |        0x0 |  R  | Read: Put a double word from the receive FIFO.                    | 

### IIS TX FIFO STREAM REGISTER @ 0x14
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-0   |        0x0 |  W  | Write: Put a double word into the transmit FIFO. Attention: Stall signal is connect to the IIS TX FIFO full flag! | 

### IIS RX FIFO STREAM REGISTER @ 0x18
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-0   |        0x0 |  R  | Read: Put a double word from the receive FIFO.Attention: Stall signal is connect to the IIS RX FIFO empty flag! | 

### IIS FIFO FILL LEVEL @ 0x1C
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-16  |        0x0 |  R  | IIS RX FIFO fill level |
| 15-0   |        0x0 |  R  | IIS TX FIFO fill level |
