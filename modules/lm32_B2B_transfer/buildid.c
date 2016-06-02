#define BUILDID __attribute__((section(".buildid")))
const char BUILDID build_id_rom[] = "\
UserLM32\n\
Project     : \n\
Version     : 1.0.0\n\
Platform    : \n\
Build Date  : Do Jun 02 09:58:36 CEST 2016\n\
Prepared by : jbai Jiaoni <J.bai@gsi.de>\n\
Prepared on : belpc136\n\
OS Version  : Debian GNU/Linux 7.10 (wheezy)  Linux 3.2.0-4-amd64 x86_64\n\
GCC Version : lm32-elf-gcc (GCC) 4.5.3\n\
FW-ID ROM will contain:\n\n\
   56657e8 build: fixed search for project name\n\
   0e68e48 build: fixed regex to get top manifest path from hdlmake\n\
   0b80c42 build: back to old local hdl-make call\n\
   f158f2c Merge new msi system into proposed_master\n\
   9f2d761 eca channels: changed constants\n\
";
