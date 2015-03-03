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
| 3-0    |        0x0 |  R  | Shows status of the push button                                   | 

### Clock Control Register @ 0x8

| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| 31-1   |        0x0 |  R  | Reserved                                                          | 
| 3-0    |        0x0 | R/W | Control external clock enable                                     | 

## Code Generation

The wishbone slave code was generated with wbgenplus. 
You can get it here: https://github.com/mkreider/wbgenplus

Example usage:

    python wbgenplus.py pmc_ctrl.xml


