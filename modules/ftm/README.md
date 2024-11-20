# Datamaster Firmware, Library and Tools
The ftm module contains LM32 firmware and for the host the libcarpedm
and tools to control the datamaster and to manipulate the schedules.

# Components
## LM32 Firmware
ftm.bin is the LM32 firmware. libcarpedm connects to the firmware via
etherbone. The version of the firmware must be compatible to the version
of the library.

## libcarpedm
libcarpedm.so is the datamaster library. It is linked to the tools and
used by FESA classes.

## Tools
### dm-cmd
dm-cmd is used to control the datamaster from command line. It is mainly
a tool for development and debugging error situations.

### dm-sched
dm-sched is used to manipulate the schedules in the datamaster from
command line. It is mainly a tool for development and debugging error
situations.

### Other Tools
There are some more tools to analyse schedules and to convert the
command history of a datamaster.

CommandsHistory.py: convert a command log from a datamaster into a
sequence of commands. This is used to replay the commands for
debugging.

scheduleCompare: compare two schedules. This is used to check schedules
if they represent the same schedule even if the text representation is
different.

replaceChain: contract chains in a schedule to a new node replacing the
chain. This is used to simplyfy large schedules and to allow structure
analysis.

## Tests
The folder tests/ contains Python3 (pytest) based automated tests for
the tools, the library, and the firmware.

# Build Instructions
* Use make in the folder modules/ftm/ftmx86/ to build the library and the tools.
* Use make in the folder modules/ftm/tests/ to run the automated tests.
* Use make in the folder modules/ftm/analysis/scheduleCompare/main to build
scheduleCompare and replaceChain.
* Use make ftm.bin in the folder syn/gsi_pexarria5/ftm/ to build the LM32
firmware ftm.bin.

# Version History

| Commit     | Date       | Tool version | Firmware version | libcarpedm version |
| --- | --- | --- | --- | --- |
| 74c8bbc4c4 | 2023-08-23 |              |                  | 1.0.0 |
|||||libcarpedm.so with version number in filename and links 
| 6578ecdff3 | 2022-06-01 | 0.36.3       | 8.0.4            | |
| 81f4ffaaf2 | 2022-03-02 |              | 8.0.3            | |
| 91ddd4de16 | 2021-07-21 | 0.36.0       | 8.0.0            | |
| c0676086c2 | 2020-10-20 | 0.34.0       | 7.0.1            | |
| 130c6816ce | 2020-04-29 | 0.33.5       | 6.2.2            | |
| 535a7d603e | 2019-09-26 | 0.33.4       |                  | |
| c9a95855fa | 2019-09-13 | 0.33.2       |                  | |
| dc7551702e | 2019-08-26 | 0.33.0       |                  | |
| fa8496d6f9 | 2019-08-14 | 0.32.1       |                  | |
| 5585d1b738 | 2019-06-03 | 0.32.0       | 6.2.0            | |
| a4f0e569b7 | 2019-05-22 | 0.31.0       |                  | |
| 4074515bc6 | 2019-03-18 | 0.30.0       |                  | |
| a3e8de6a81 | 2019-03-18 |              | 6.1.3            | |
| 15f42702c1 | 2019-02-01 | 0.29.5       | 6.1.0            | |
| d500cd9722 | 2018-08-20 | 0.27.1       | 6.0.0            | |
| 73a30a6e11 | 2018-04-27 |              | 3.0.0            | |
| 30a4a55406 | 2018-04-27 | 0.18.0       |                  | |
| 14d5dea052 | 2018-04-16 | 0.16.0       |                  | |
| f3b22889ee | 2018-04-11 |              | 2.2.0            | |
| 66620dfba3 | 2018-04-06 | 0.15.2       |                  | |
| 2f5f144238 | 2018-03-28 | 0.15.0       | 2.1.0            | |
| bdc6bf08d9 | 2018-03-16 | 0.14.0       |                  | |
| bc9018e42b | 2018-03-07 | 0.12.0       |                  | |
| ae87795cbe | 2018-02-16 | 0.10.1       |                  | |
| 4475d34ef5 | 2018-02-06 | 0.10.0       |                  | |
| 04ca6d8cba | 2018-01-30 | 0.9.0        |                  | |
| 66926f1545 | 2018-01-19 | 0.8.0        |                  | |
| 186548b04a | 2017-12-19 | 0.7.0        |                  | |
| cb8c8e977c | 2016-08-03 | 0.6.3        |                  | |
| 60007eb0f5 | 2016-07-22 | 0.1.4        |                  | |
