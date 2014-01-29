volatile unsigned int* scu_reg = 0x100800;
void main() {
  *(scu_reg) = 0xdeadbeef;
  while(1);
}
