# SSD1325 Serial Driver with Wishbone Interface

---

##Contents

* Synopsis
* Characters
* Register Layout

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

## Characters

The driver (c-file) can display all ASCII characters from 0x20 to 0x7E. Each character is consists of a 8x6 pixel array.
Every character is printed with this order:

<pre>
[00][01][16][17][32][33]
[02][03][18][19][34][35]
[04][05][20][21][36][37]
[06][07][22][23][38][39]
[08][09][24][25][40][41]
[10][11][26][27][42][43]
[12][13][28][29][44][45]
[14][15][30][31][46][47]
</pre>

Example for character 'A':

Byte array: {0x0f,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0xff,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0x00}

Output:

<pre>
[ ][#][#][#][ ][ ]
[#][ ][ ][ ][#][ ]
[#][ ][ ][ ][#][ ]
[#][#][#][#][#][ ]
[#][ ][ ][ ][#][ ]
[#][ ][ ][ ][#][ ]
[#][ ][ ][ ][#][ ]
[ ][ ][ ][ ][ ][ ]
</pre>

Each nibble (half byte) will set one pixel.

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
