#include "dbg.h"


void strreverse(char* begin, char* end) {
   
   char aux;
   while(end>begin) aux=*end, *end--=*begin, *begin++=aux;
}
   
void itoa(int value, char* str, int base) {
   
   static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
   char* wstr=str;
   int sign;
   int res;

   // Validate base
   if (base<2 || base>35){ *wstr='\0'; return; }
   
   // Take care of sign
   if ((sign=value) < 0) value = -value;
   
   // Conversion. Number is reversed.
   do {
      res = value / base;
      *wstr++ = num[value % base];
      value = res;
   }while(value);
   
   if(sign<0) *wstr++='-';
   *wstr='\0';
   
   // Reverse string
   strreverse(str,wstr-1);
}

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
       mprintf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               mprintf ("  %s\n", buff);

            // Output the offset.
           mprintf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       mprintf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
}
