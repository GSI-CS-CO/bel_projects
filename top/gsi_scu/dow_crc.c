#include <dow_crc.h>

char calc_crc(int id_h, int id_l) {
  unsigned char crc = 0;
  int i;
  // crc = table[crc xor byte]
  // begin with LSB
  for (i = 0; i <= 24; i+=8) {
    crc = dow_crc_table[crc ^ (((id_l >> i) & 0xff))];
  }
  for (i = 0; i <= 24; i+=8) {
    crc = dow_crc_table[crc ^ (((id_h >> i) & 0xff))];
  }
  return crc;
}

