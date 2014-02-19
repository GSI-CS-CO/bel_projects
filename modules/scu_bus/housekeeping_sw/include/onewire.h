#ifndef __ONEWIRE_H
#define __ONEWIRE_H

void ow_init();
uint32_t ow_reset(uint32_t port);
uint8_t ow_read_byte(uint32_t port);
int8_t ow_write_byte(uint32_t port, uint32_t byte);
int ow_write_block(int port, uint8_t *block, int len);
int ow_read_block(int port, uint8_t *block, int len);

int8_t ds18x_read_serial(uint8_t *id);
int8_t ds18x_read_temp(uint8_t *id, int *temp_r);
int ds18x_init();

#endif
