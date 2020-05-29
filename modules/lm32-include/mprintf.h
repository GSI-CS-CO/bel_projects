/*!
 * @file mprintf.c
 * @brief implementation of the mprintf- family.
 *
 * @date unknown (improved 28.05.2020)
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author unknown (improved by Ulrich Becker <u.becker@gsi.de>)
 */
#ifndef _MPRINTF_H
#define _MPRINTF_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef __lm32__
  #include "uart.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int vprintf( char const *format, va_list ap );
//int _p_vsprintf(char const *format,va_list ap, char* dst);
int vsnprintf( char* s, size_t n, const char* format, va_list arg );
int mprintf(char const *format, ...);
int sprintf(char *dst, char const *format, ... );
int snprintf ( char * s, size_t n, const char * format, ... );
#define C_DIM 0x80
void m_cprintf(int color, const char *fmt, ...);
void m_pcprintf(int row, int col, int color, const char *fmt, ...);
void m_term_clear();

#ifdef __cplusplus
}
#endif

#endif /* ifndef _MPRINTF_H */
/*================================== EOF ====================================*/
