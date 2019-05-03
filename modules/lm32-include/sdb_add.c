unsigned int sdb_add(void)
{
    unsigned ret;
    __asm__ __volatile__(".long   0x91600800" : "=r" (ret) : :);
    return ret;
}
