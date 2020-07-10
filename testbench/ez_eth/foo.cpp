
//#define _GNU_SOURCE
#include "file_access.h"
#include <ctype.h>
#include <stdio.h>

#include <queue>
#include <iostream>


int main(int argc, char *argv[])
{
  
  file_access_init(0);
  printf("Started tap2\n");

  /* Now read data coming from the kernel */
  while(1) {
    int p = file_access_fetch_packet();

    int w = file_access_read();
    if(w >= 0) {std::cout << '\t' << w; }
    else if(w == PACKET_EMPTY) {std::cout << std::endl;}
  }  

}  