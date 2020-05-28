/*!
 * @file mprintf.c
 * @brief implementation of the mprintf- family.
 *
 * @date unknown (improved 28.05.2020)
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author unknown (improved by Ulrich Becker <u.becker@gsi.de>)
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <helper_macros.h>

#ifdef __lm32__
  #include "uart.h"
#else
  /*
   * Makes it possible to debug as normal PC- application.
   */
  void uart_write_byte( int c )
  {
     putch( c );
  }
#endif

/*!
 * @brief Type declaration of the character output function.
 */
typedef void (*PUTCH_F)(int x);

/*!
 * @brief Helper object for target string.
 *        Will used from sprintf and snprintf.
 */
typedef struct
{
   char*   pStart;
   char*   pCurrent;
   size_t  limit;
} STRING_T;

STRING_T g_target;

/*! --------------------------------------------------------------------------
 * @brief Adds a single character to the target string.
 *        Will used from sprintf and snprintf.
 */
STATIC void addToString( int c )
{
   *g_target.pCurrent++ = (char)c;
}


#ifndef DEFAULT_SPRINTF_LIMIT
 #define DEFAULT_SPRINTF_LIMIT 80
#endif

/*! ---------------------------------------------------------------------------
 * @brief Makes the output of a single character either via UART or string.
 *
 * In the case of string-output a limit will observed as well.
 */
#define __PUT_CHAR( c )                                            \
{                                                                  \
   if( (__putch == addToString) &&                                 \
       ((g_target.pCurrent - g_target.pStart) > g_target.limit ) ) \
   {                                                               \
      __putch( '\0' );                                             \
      return ret;                                                  \
   }                                                               \
   ret++;                                                          \
   __putch( c );                                                   \
}

/*! ---------------------------------------------------------------------------
 * @brief Base function for all printf variants.
 * @param __putch Pointer to the character output function.
 */
STATIC int vprintfBase( PUTCH_F __putch, char const *format, va_list ap )
{
   int ret = 0;
   while( true )
   {
      unsigned char  scratch[16];
      unsigned char  format_flag;
      unsigned int   u_val = 0;
      unsigned char  base;
      unsigned char* ptr;
      unsigned char  width = 0;
      unsigned char  fill;
      unsigned char  hexOffset;
      bool           signum;

      while( (format_flag = *format++) != '%' )
      {
         if( format_flag == '\0' )
         {
            va_end( ap );
            if( __putch == addToString )
               __putch( '\0' );
            return ret;
         }
         __PUT_CHAR( format_flag );
      }

      /*
       * check for zero padding
       */
      format_flag = *format - '0';
      if( format_flag == 0 )
      {
         fill = '0';
         format++;
      }
      else
      {
         fill = ' ';
      }

      /*
       * check for width spec
       */
      format_flag = *format - '0';
      if( format_flag > 0 && format_flag <= 9 )
      {
         width = format_flag;
         format++;
      }
      else
      {
         width = 0;
      }

      switch( format_flag = *format++ )
      {
         case 'S':
         case 's':
            ptr = (unsigned char*)va_arg( ap, char* );
            while( *ptr != '\0' )
               __PUT_CHAR( *ptr++ );
            continue;

         case 'i':
         case 'd':
            signum = true;
            base = 10;
            break;

         case 'u':
            signum = false;
            base = 10;
            break;

         case 'x':
            base = 16;
            hexOffset = 'a' - '9' - 1;
            break;

         case 'X':
         case 'p':
            base = 16;
            hexOffset = 'A' - '9' - 1;
            break;

         case 'c':
            format_flag = va_arg( ap, int );
            /* No break here! */
         default:
            __PUT_CHAR( format_flag );
            continue;
      }

      u_val = va_arg( ap, unsigned int );
      if( signum && (u_val & (1 << (BIT_SIZEOF(u_val)-1)) ) != 0 )
      {
         __PUT_CHAR('-');
         u_val = -u_val;
      }

      ptr = scratch + ARRAY_SIZE(scratch);
      *--ptr = '\0';

      do
      {
         char ch = (u_val % base) + '0';
         if( ch > '9' )
            ch += hexOffset;

         *--ptr = ch;
         u_val /= base;

         if( width != 0 )
            width--;
      }
      while( u_val > 0 );

      while( width-- != 0 )
         *--ptr = fill;

      while( *ptr != '\0' )
         __PUT_CHAR( *ptr++ );
   } /* end while( true ) */
   return ret;
}

/*! ---------------------------------------------------------------------------
 */
int vprintf( char const *format, va_list ap )
{
   return vprintfBase( uart_write_byte, format, ap );
}

/*! ---------------------------------------------------------------------------
 */
STATIC int _p_vsprintf( char const *format, va_list ap, char* dst, const size_t n )
{
   g_target.pCurrent = dst;
   g_target.pStart   = dst;
   g_target.limit    = n;
   return vprintfBase( addToString, format, ap );
}

/*! ---------------------------------------------------------------------------
 */
int mprintf( char const *format, ... )
{
   int rval;
   va_list ap;
   va_start( ap, format );
   rval = vprintf( format, ap );
   va_end( ap );
   return rval;
}

/*! ---------------------------------------------------------------------------
 */
int sprintf( char* dst, char const *format, ... )
{
   va_list ap;
   va_start( ap, format );
   const int r = _p_vsprintf( format, ap, dst, DEFAULT_SPRINTF_LIMIT );
   va_end( ap );
   return r;
}

/*! ---------------------------------------------------------------------------
 */
int snprintf( char* s, size_t n, const char * format, ... )
{
   va_list ap;
   va_start( ap, format );
   const int r = _p_vsprintf( format, ap, s, n );
   va_end( ap );
   return r;
}

/*! ---------------------------------------------------------------------------
 */
#define C_DIM 0x80
void m_cprintf( int color, const char *fmt, ... )
{
   va_list ap;
   mprintf( "\033[0%d;3%dm", color & C_DIM ? 2:1, color & 0x7f);
   va_start( ap, fmt );
   vprintf( fmt, ap );
   va_end( ap );
}

/*! ---------------------------------------------------------------------------
 */
void m_pcprintf( int row, int col, int color, const char *fmt, ... )
{
   va_list ap;
   mprintf( "\033[%d;%df", row, col );
   mprintf( "\033[0%d;3%dm",color & C_DIM ? 2:1, color & 0x7f );
   va_start( ap, fmt );
   vprintf( fmt, ap );
   va_end( ap );
}

/*! ---------------------------------------------------------------------------
 */
void m_term_clear( void )
{
   mprintf("\033[2J\033[1;1H");
}

/*================================== EOF ====================================*/
