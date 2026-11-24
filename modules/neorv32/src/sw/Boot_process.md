# Boot Procedure

1. Write bootloader under `bel_project_2204/uart_upload1305/bel_projects/ip_cores/neorv32/sw/bootloader/main.c`
2. Go to the path `bel_project_2204/uart_upload1305/bel_projects/ip_cores/neorv32/sw/bootloader`
3. Execute `make clean bootloader`, and then neorv32_bootrom_image.vhd will be updated.
4. Synthesis all project and upload to Pexp:

   ```
   make pexp-neorv32-clean
   make pexp-neorv32
   ll ./syn/gsi_pexp/neorv32/pexp_neorv32.rpd    #check the update time
   eb-flash dev/ttyUSB0 ./syn/gsi_pexp/neorv32/pexp_neorv32.rpd
   ```
5. Repower Pexp.
6. Open GTKTerm, port: dev/ttyACM0, Baudrate: refer to `bel_project_2204/uart_upload1305/bel_projects/ip_cores/neorv32/sw/bootloader/config.h`, current setting is neorv32 default value 19200.

# Update program during run time

1. Go to the path `bel_project_2204/uart_upload1305/bel_projects/modules/neorv32/src/sw`, the eb tool is upload_test.sh, modified from eb-neorv32-ctl
2. Execute `bash upload_test.sh dev/ttyUSB0 -d sim_fifo_boot/program.bin`, -d is to (eb)put program.bin to DPRAM 0x71000000. And then the bootloader will copy the program inside DPRAM to IMEM 0x90000000, and then restart CPU to execute program from IMEM.
3. Or execute `bash upload_test.sh dev/ttyUSB0 -f sim_fifo_boot/program.bin`, -f is to (eb)put/(eb)write program.bin to FIFO WRite port 0x4000800. And then the bootloader will read the program from FIFO ReaD port to IMEM 0x90000000, and then restart CPU to execute program from IMEM.

# Bootloader Workflow

1. Set GPIO0 to low after system_setup, make sure the wishbone adapter works.
2. Set GPIO1 to high to access FIFO status.
3. Polling until fifo gets the 1st data: file size of program.bin
   !! The sequence is read file size first and then read boot register, cannot be changed, to make sure when checking boot register, the value is already set, instead of initial value in register. !!
4. Check the FIFO boot register, 0: DPRAM, 1: FIFO.
   * If DPRAM, set GPIO1 to 0, disconnect FIFO, connect DPRAM to Neorv32
   * If FIFO, remain GPIO1 setting
5. Read out data and write into IMEM word by word.
6. Inline Assembly to synchronize memory and cache, from original bootloader.
7. Restart CPU.

# Eb tool Workflow

#### Upload through DPRAM -d

1. restart neorv32, so that neorv32 can load into Bootloader
2. Write 0x0 to FIFO Boot Register, indicate bootloader to load data from DPRAM
3. Write the file size (program.bin) to FIFO
   !! The sequence is write FIFO Boot setting and then write file size, cannot be changed, to make sure when the file size received by bootloader, boot register is already set. !!
4. eb put the program to DPRAM, the following are the same as in eb-neorv32-ctl tool.


### Upload through FIFO -f

1. Same
2. Write 0x1 to FIFO Boot Register, indicate bootloader to load data from FIFO
3. Same
4. eb-write word by word into fifo or eb-put segment by segment into fifo, the segment size should be the same as fifo size, current setting is 512 byte. !! NOT VERIFIED YET!!


# Current Error

1. Execute `bash upload_test.sh dev/ttyUSB0 -d sim_fifo_boot/program.bin` or `bash upload_test.sh dev/ttyUSB0 -f sim_fifo_boot/program.bin`, it works and will show this message, but the problem is: now using all fifo write address for read function in Bootloader.

   ```
   #define FIFO_RD_BASE_ADDRESS ((volatile int *)0x04000800)   
   #define FIFO_RD_STATUS_ADDR ((volatile int *)0x04000804)
   #define RD_BOOT_STATUS_ADDR ((volatile int *)0x04000808)
   ```

   ```
   1
   Hi here is bootloader
   wait for first input
   wait for first input
   wait for first input
   get file_size0x000017A0switch to dpram
   start load data in dpram to IMEM
   endOK
   Booting ...

   PC at:9000007c

    Here is main.c, success!PC at:900000b8

   ```
