#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "display.h"
#include "irq.h"
#include "scu_bus.h"
#include "aux.h"
#include "mini_sdb.h"
#include "board.h"
#include "uart.h"
#include "w1.h"
#include "fg.h"
#include "cb.h"
//#define DEBUG
//#define FGDEBUG
//#define CBDEBUG
extern struct w1_bus wrpc_w1_bus;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED board_id = -1;
uint16_t SHARED board_temp = -1;
uint64_t SHARED ext_id = -1;
uint16_t SHARED ext_temp = -1;
uint64_t SHARED backplane_id = -1;
uint16_t SHARED backplane_temp = -1;
uint32_t SHARED fg_magic_number = 0xdeadbeef;
uint32_t SHARED fg_version = 0x1;
struct circ_buffer SHARED fg_buffer[MAX_FG_DEVICES];
struct scu_bus SHARED scub;
struct fg_list SHARED fgs;
volatile uint32_t SHARED fg_control;
volatile unsigned short* scub_base;
volatile unsigned int* BASE_ONEWIRE;
volatile unsigned int* BASE_UART;
volatile unsigned int* scub_irq_base;
volatile unsigned int* lm32_irq_endp;
int slaves[MAX_SCU_SLAVES+1] = {0};
volatile unsigned short icounter[MAX_SCU_SLAVES+1];
void usleep(int us)
{
unsigned i;
unsigned long long delay = us;
/* prevent arithmetic overflow */
delay *= CPU_CLOCK;
delay /= 1000000;
delay /= 4; // instructions per loop
for (i = delay; i > 0; i--) asm("# noop");
}
void msDelay(int msecs) {
usleep(1000 * msecs);
}
void show_msi()
{
char buffer[12];
mat_sprinthex(buffer, global_msi.msg);
disp_put_str("D ");
disp_put_str(buffer);
disp_put_c('\n');
mat_sprinthex(buffer, global_msi.adr);
disp_put_str("A ");
disp_put_str(buffer);
disp_put_c('\n');
mat_sprinthex(buffer, (unsigned long)global_msi.sel);
disp_put_str("S ");
disp_put_str(buffer);
disp_put_c('\n');
}
void isr1()
{
char buffer[12];
struct pset *p;
unsigned char slave_nr = global_msi.adr>>2 & 0xf;
unsigned short tmr_irq_cnts = scub_base[((slave_nr) << 16) + TMR_BASE + TMR_IRQ_CNT];
disp_put_c(slave_nr + '0');
disp_put_c(' ');
sprinthex(buffer, icounter[slave_nr - 1], 4);
disp_put_str(buffer);
disp_put_c('\n');
disp_put_c(' '); disp_put_c(' ');
sprinthex(buffer, tmr_irq_cnts, 4);
disp_put_str(buffer);
disp_put_c('\n');
if ((tmr_irq_cnts == icounter[slave_nr - 1])) {
scub_base[((slave_nr) << 16) + SLAVE_INT_ACT] |= 5; //ack timer and powerup irq
}
icounter[slave_nr-1]++;
mprintf("irq1\n");
//show_msi();
//usleep(500000);
//mprintf("adr: 0x%x\n", (unsigned long)global_msi.adr);
//mprintf("msg: 0x%x\n", (unsigned long)global_msi.msg);
}
void init_irq() {
int i;
//SCU Bus Master
scub_base[GLOBAL_IRQ_ENA] = 0x20; //enable slave irqs
scub_irq_base[0] = 0x1; // reset irq master
scub_irq_base[2] = 0xfff; //enable all slaves
for (i = 0; i < 12; i++) {
scub_irq_base[8] = i; //channel select
scub_irq_base[9] = 0x08154711; //msg
scub_irq_base[10] = 0x200300 + ((i+1) << 2); //destination address, do not use lower 2 bits
}
isr_table_clr();
isr_ptr_table[1] = &isr1;
irq_set_mask(0x02);
irq_enable();
mprintf("MSI IRQs configured.\n");
}
void dis_irq() {
int i = 0;
irq_set_mask(0x02);
irq_disable();
isr_table_clr();
for (i = 0; i < MAX_SCU_SLAVES; i++) {
icounter[i] = 0; //reset counter in ISR
}
}
void configure_slaves() {
int i = 0;
scub_base[SRQ_ENA] = 0x0; //reset bitmask
scub_base[MULTI_SLAVE_SEL] = 0x0; //reset bitmask
while(slaves[i]) {
disp_put_c('x');
scub_base[SRQ_ENA] |= (1 << (slaves[i]-1)); //enable irqs for the slave
scub_base[MULTI_SLAVE_SEL] |= (1 << (slaves[i]-1)); //set bitmask for broadcast select
scub_base[(slaves[i] << 16) + SLAVE_INT_ENA] = 0x4; //enable tmr irq in slave macro
scub_base[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
scub_base[(slaves[i] << 16) + TMR_BASE + TMR_VALUEL] = 0xffff; //enable generation of tmr irqs
scub_base[(slaves[i] << 16) + TMR_BASE + TMR_VALUEH] = 0x005f; //enable generation of tmr irqs
scub_base[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] |= 0x2; //enable generation of tmr irqs
i++;
}
}
void reset_slaves() {
int i = 0;
scub_base[SRQ_ENA] = 0x0; //reset bitmask
scub_base[MULTI_SLAVE_SEL] = 0x0; //reset bitmask
while(slaves[i]) {
disp_put_c('x');
scub_base[(slaves[i] << 16) + TMR_BASE + TMR_CNTRL] = 0x1; //reset TMR
i++;
}
}
void updateTemps() {
#ifdef DEBUG
mprintf("Onboard Onewire Devices:\n");
#endif
//conflicts with WR
// BASE_ONEWIRE = (unsigned int*)BASE_OW_WR;
// wrpc_w1_init();
// ReadTempDevices(0, &board_id, &board_temp);
#ifdef DEBUG
mprintf("External Onewire Devices:\n");
#endif
//BASE_ONEWIRE = (unsigned int*)BASE_OW_EXT;
wrpc_w1_init();
ReadTempDevices(0, &ext_id, &ext_temp);
ReadTempDevices(1, &backplane_id, &backplane_temp);
}
void init() {
int i=0, j;
uart_init_hw();
init_irq();
updateTemps();
init_buffers(&fg_buffer);
#ifdef CBDEBUG
for (i=0; i < MAX_FG_DEVICES; i++) {
mprintf("cb[%d]: isEmpty = %d\n", i, cbisEmpty((struct circ_buffer *)&fg_buffer, i));
mprintf("cb[%d]: isFull = %d\n", i, cbisFull((struct circ_buffer *)&fg_buffer, i));
mprintf("cb[%d]: getCount = %d\n", i, cbgetCount((struct circ_buffer *)&fg_buffer, i));
}
#endif
scan_scu_bus(&scub, backplane_id, scub_base);
scan_for_fgs(&scub, &fgs);
//probe_scu_bus(scub_base, 55, 3, slaves);
configure_slaves();
#ifdef FGDEBUG
mprintf("ID: 0x%08x%08x\n", (int)(scub.unique_id >> 32), (int)scub.unique_id);
while(scub.slaves[i].unique_id) { /* more slaves in list */
mprintf("slaves[%d] ID: 0x%08x%08x\n",i, (int)(scub.slaves[i].unique_id>>32), (int)scub.slaves[i].unique_id);
mprintf("slave macro: 0x%x\n", scub.slaves[i].version);
mprintf("CID system: %d\n", scub.slaves[i].cid_sys);
mprintf("CID group: %d\n", scub.slaves[i].cid_group);
mprintf("slot: %d\n", scub.slaves[i].slot);
j = 0;
while(scub.slaves[i].devs[j].version) { /* more fgs in list */
mprintf(" fg[%d], version 0x%x \n", j, scub.slaves[i].devs[j].version);
j++;
}
i++;
}
mprintf("fg_list-------------------\n");
i = 0;
while(fgs.devs[i]) {
mprintf("fgs.devs[%d] 0x%x\n", i, fgs.devs[i]);
mprintf("fg[%d]: version 0x%x\n", i, fgs.devs[i]->version);
mprintf(" dev_number 0x%x\n", fgs.devs[i]->dev_number);
mprintf(" offset 0x%x\n", fgs.devs[i]->offset);
mprintf(" endvalue 0x%x\n", fgs.devs[i]->endvalue);
i++;
}
#endif
/* reset_slaves();
usleep(1000);
dis_irq();
usleep(1000000);
init_irq();
usleep(1000);
configure_slaves();
*/
}
int main(void) {
char buffer[20];
int i = 0;
unsigned int favg[MAX_FG_DEVICES] = {0};
unsigned int count[MAX_FG_DEVICES] = {0};
unsigned int min_count[MAX_FG_DEVICES];
unsigned int old[MAX_FG_DEVICES] = {0};
struct param_set pset;
/* init min_count */
for(i = 0; i < MAX_FG_DEVICES; i++) min_count[i] = BUFFER_SIZE-2;
discoverPeriphery();
scub_base = (unsigned short*)find_device_adr(GSI, SCU_BUS_MASTER);
BASE_ONEWIRE = (unsigned int*)find_device_adr(CERN, WR_1Wire);
scub_irq_base = (unsigned int*)find_device_adr(GSI, SCU_IRQ_CTRL);
//lm32_irq_endp = (unsigned int*)find_device_adr(GSI, IRQ_ENDPOINT);
disp_reset();
disp_put_c('\f');
init();
mprintf("scub_irq_base is: 0x%x\n", scub_irq_base);
//mprintf("irq_endp is: 0x%x\n", lm32_irq_endp);
while(1);
//while(1) {
// mprintf("global_msi: 0x%x\n", &global_msi);
// _irq_entry();
//}
/* wait until buffers are filled */
for(i = 0; i < MAX_FG_DEVICES; i++) {
while(cbgetCount((struct circ_buffer *)&fg_buffer, i) < BUFFER_SIZE - 5) usleep(10000);
}
while(1) {
//updateTemps();
for(i = 0; i < MAX_FG_DEVICES; i++) {
if (!cbisEmpty((struct circ_buffer *)&fg_buffer, i)) {
cbRead((struct circ_buffer *)&fg_buffer, i, &pset);
if((pset.coeff_c % 1000) == 0) {
mprintf("cb[%d]: fcnt: %d fmin: %d favg: %d coeff_a: %x coeff_b: 0x%x coeff_c: 0x%x\n",
i, count[i], min_count[i], favg[i]/1000, pset.coeff_a, pset.coeff_b, pset.coeff_c);
favg[i] = 0;
}
if (old[i] + 1 != pset.coeff_c) {
mprintf("cb[%d]: buffer value not consistent old: %x coeff_c: %x\n", i, old[i], pset.coeff_c);
cbDump((struct circ_buffer *)&fg_buffer, i);
return(1);
}
old[i] = pset.coeff_c;
}
count[i] = cbgetCount((struct circ_buffer *)&fg_buffer, i);
favg[i] += count[i];
if (count[i] < min_count[i]) min_count[i] = count[i];
if (count[i] == 0) {
mprintf("cb[%d] fcnt: %d\n", i, count[i]);
cbDump((struct circ_buffer *)&fg_buffer, i);
return(1);
}
}
usleep(800);
//placeholder for fg software
//if (fg_control) {
// init();
// fg_control = 0;
//}
}
return(0);
}

