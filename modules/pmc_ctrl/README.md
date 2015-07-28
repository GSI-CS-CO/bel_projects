# PMC CONTROL UNIT

---

##Contents

* Synopsis
* Register Layout
* Code Generation

## Synopsis

## Register Layout

### Hex Switch Register @ 0x0

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-4   |        0x0 |  R  | Reserved                                                          | 
| 3-0    |        0x0 |  R  | Shows hex switch inputs                                           | 

### Push Button Register @ 0x4

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-1   |        0x0 |  R  | Reserved                                                          | 
| 0      |        0x0 |  R  | Shows status of the push button                                   | 

### Hex Switch (CPLD) Register @ 0x8

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-4   |        0x0 |  R  | Reserved                                                          | 
| 3-0    |        0x0 |  R  | Shows hex switch inputs (CPLD)                                    | 

### Push Button (CPLD) Register @ 0xC

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-1   |        0x0 |  R  | Reserved                                                          | 
| 0      |        0x0 |  R  | Shows status of the push button (CPLD)                            | 

### Clock Control Register @ 0x10

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-1   |        0x0 |  R  | Reserved                                                          | 
| 0      |        0x0 | R/W | External input clock output enable                                | 

### Logic Control Register @ 0x14

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-17  |        0x0 |  R  | Reserved                                                          | 
| 16-0   |        0x0 | R/W | External logic analyzer output enable                             | 

### Logic Control Output Register @ 0x18

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-17  |        0x0 |  R  | Reserved                                                          | 
| 16-0   |        0x0 | R/W | External logic analyzer output (write)                            | 

### Logic Control Output Register @ 0x1c

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-17  |        0x0 |  R  | Reserved                                                          | 
| 16-0   |        0x0 |  R  | External logic analyzer input (read)                              | 

## Code Generation

The wishbone slave code was generated with wbgenplus. 
You can get it here: https://github.com/mkreider/wbgenplus

Example usage:

    python wbgenplus.py pmc_ctrl.xml


