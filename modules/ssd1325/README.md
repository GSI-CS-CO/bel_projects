# SSD1325 Serial Driver with Wishbone Interface

---

##Contents

* Synopsis
* Register Layout
* Characters
* Files

---

## Synopsis

This unit is used to control all display types who have a SSD1325 controller interface. 
The driver is designed to use the serial mode (similar to SPI). Only five pins are needed:

* Serial Clock (SCLK)
* Serial Data (SD)
* Slave Select (SS)
* Data/Control (DC)
* Reset (RST)

A wishbone interface with four registers is provided by this unit. For details see section "Register Layout".

---

## Register Layout

### TX FIFO DATA REGISTER @ 0x00
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-8   |        0x0 |  R  | -Reserved-                                                        | 
|  7-0   |        0x0 |  W  | Write: Put a byte into the transmit FIFO.                         | 

### TX FIFO STATUS REGISTER @ 0x04

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-3   |        0x0 |  R  | -Reserved-                                                        | 
| 2      |        0x0 |  R  | Transmit FIFO full flag.                                          |
| 1      |        0x0 |  R  | Transmit FIFO empty flag.                                         | 
| 0      |        0x0 |  R  | Last transmission done flag.                                      | 

### TX FIFO FILL LEVEL REGISTER @ 0x08

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-8   |        0x0 |  R  | -Reserved-                                                        | 
| 7-0    |        0x0 |  R  | Transmit FIFO fill level.                                         |

### CONTROL REGISTER @ 0x0C

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-6   |        0x0 |  R  | -Reserved-                                                        | 
| 5      |        0x0 | R/W | Read: Get value. Write: Clear pending interrupt(0x1). |
| 4      |        0x0 | R/W | Read: Get value. Write: Enable(0x1) or disable(0x0) interrupt. |
| 3      |        0x0 | R/W | Read: Get value. Write: Control slave select pin directly: 0x1 => High; 0x0 => Low. |
| 2      |        0x0 | R/W | Read: Get value. Write: Enable(0x1) or disable(0x0) manual slave select driving. If this bit is set to 0x1, the slave select output will become the value of bit 3
| 1      |        0x0 | R/W | Read: Get value. Write: Drive DC to high or low (depending on this bit value). |
| 0      |        0x0 | R/W | Read: Get value. Write: Drive RST to high or low (depending on this bit value). |

---

## Characters

The driver (c-file) can display all ASCII characters from 0x20 to 0x7E. Each character is consists of a 8x6 pixel array.
Every character is printed with this order:

[00][08][16][24][32][40]<br>
[01][09][17][25][33][41]<br>
[02][10][18][26][34][42]<br>
[03][11][19][27][35][43]<br>
[04][12][20][28][36][44]<br>
[05][13][21][29][37][45]<br>
[06][14][22][30][38][46]<br>
[07][15][23][31][39][47]<br>

Example for character 'A':

Byte array: {0x0f,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0xff,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0x00}

Output:

[ ][#][#][#][ ][ ]<br>
[#][ ][ ][ ][#][ ]<br>
[#][ ][ ][ ][#][ ]<br>
[#][#][#][#][#][ ]<br>
[#][ ][ ][ ][#][ ]<br>
[#][ ][ ][ ][#][ ]<br>
[#][ ][ ][ ][#][ ]<br>
[ ][ ][ ][ ][ ][ ]<br>

Each nibble (half byte) will set one pixel.

--- 

## Files

### HDL Files

* generic_fifo(_pkg).vhd -> Used as FIFO between wishbone interface and serial master
* generic_serial_master(_pkg).vhd -> Serial master, will control clock, data and slave select
* wb_ssd1325_serial_driver(_pkg).vhd -> Main wishbone interface, used to control reset, data/command and slave select (depending on setup)
* ssd1325_serial_driver_test_bench.vhd -> Simple test bench for the complete unit

### C Files

* ssd1325_serial_driver.h -> Header file for c driver functions
* ssd1325_serial_driver.c -> Implementation of c driver functions

