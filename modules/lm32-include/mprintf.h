#ifndef  __MPRINTF_H
#define __MPRINTF_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "uart.h"

int vprintf(char const *format,va_list ap);
static int _p_vsprintf(char const *format,va_list ap, char*dst);
int mprintf(char const *format, ...);
int sprintf(char *dst, char const *format, ...);

#define C_DIM 0x80                                                                                                    
void m_cprintf(int color, const char *fmt, ...);
void m_pcprintf(int row, int col, int color, const char *fmt, ...);
void m_term_clear();

#endif
