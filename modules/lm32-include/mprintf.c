/*!
 * @file mprintf.c
 * @brief Implementation of the mprintf- family.
 *
 * @note In contrast to the ANSI printf family the mprintf family
 *       doesn't support floating point formats!
 *
 * @date unknown (improved 28.05.2020)
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author unknown (improved by Ulrich Becker <u.becker@gsi.de>)
 */
#include <stdbool.h>
#include <stdint.h>
#include <helper_macros.h>
#include <mprintf.h>
#ifndef CONFIG_USE_LINUX_PRINTF
  #include <pp-printf.h>
#endif

#ifndef __lm32__
  #include <stdio.h>
  /*
   * Makes it possible to debug as normal PC- application.
   */
  STATIC inline ALWAYS_INLINE void uart_write_byte( const int c )
  {
     putch( c );
  }
#endif


struct _PRINTF_T;

/*!
 * @ingroup PRINTF
 * @brief Type declaration of the character output function.
 */
typedef bool (*PUTCH_F)( struct _PRINTF_T*, const int );


/*!
 * @ingroup PRINTF
 * @brief Helper object for target string.
 */
typedef struct _PRINTF_T
{
   const char*   pStart;
   char*         pCurrent;
   const size_t  limit;
   PUTCH_F       putch;
} PRINTF_T;

/*! --------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Adds a single character to the target string.
 *        Will used from sprintf and snprintf.
 * @see sprintf
 * @see snprintf
 * @param pPrintfObj Pointer to the internal printf-object.
 * @param c Character to put in the string.
 * @retval true Limit has been reached, string has been terminated.
 * @retval false Character in string copied.
 */
STATIC bool addToString( PRINTF_T* pPrintfObj, const int c )
{
   if( (pPrintfObj->pCurrent - pPrintfObj->pStart) >= pPrintfObj->limit )
   {
      *pPrintfObj->pCurrent = '\0';
      return true;
   }
   *pPrintfObj->pCurrent++ = (char)c;
   return false;
}

/*! --------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Sends a single character to the UART in the case of LM32.
 *        Will used from mprintf
 * @see mprintf
 * @param pPrintfObj Pointer to the internal printf-object (will not used).
 * @param c Character to put in the string.
 * @retval false Always
 */
STATIC bool sendToUart( PRINTF_T* pPrintfObj UNUSED, const int c )
{
   uart_write_byte( c );
   return false;
}

/*! ---------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Makes the output of a single character either via UART or string.
 *
 * @note This macro is only within function vprintfBase valid!
 */
#define __PUT_CHAR( c )                                                      \
{                                                                            \
   if( pPrintfObj->putch( pPrintfObj, c ) )                                  \
      return ret;                                                            \
   ret++;                                                                    \
}

/*! ---------------------------------------------------------------------------
 * @ingroup PRINTF
 * @brief Base function for all printf variants.
 * @param pPrintfObj->putch Pointer to the character output function.
 */
STATIC int vprintfBase( PRINTF_T* pPrintfObj, const char* format, va_list ap )
{
   /*
    * Variable ret becomes incremented within macro __PUT_CHAR
    */
   int ret = 0;

   while( true )
   {
      char currentChar;
      /*
       * Forwarding currently character until the character "%" has found.
       */
      while( (currentChar = *format++) != '%' )
      {
         if( currentChar == '\0' )
         {
            if( pPrintfObj->putch == addToString )
               pPrintfObj->putch( pPrintfObj, '\0' );
            return ret;
         }
         __PUT_CHAR( currentChar );
      }

      /*!
       * First character after "%" character.
       */
      currentChar = *format;
      /*
       * check for zero padding
       */
      unsigned char paddingChar;
      if( currentChar == '0' )
      {
         paddingChar = '0';
         format++;
         /*
          * Second character after "%" character.
          */
         currentChar = *format;
      }
      else
      {
         paddingChar = ' ';
      }

      /*
       * check for padding space
       */
      unsigned int paddingWidth;
      if( currentChar > '0' && currentChar <= '9' )
      {
         paddingWidth = currentChar - '0';
         format++;
      }
      else
      {
         paddingWidth = 0;
      }

      unsigned char* ptr;
      unsigned int hexOffset = 0;
      unsigned int base;
      bool     signum = false;
      switch( currentChar = *format++ )
      {
         case 'S': /* No break here! */
         case 's':
            ptr = (unsigned char*)va_arg( ap, char* );
            while( *ptr != '\0' )
               __PUT_CHAR( *ptr++ );
            continue;

         case 'i': /* No break here! */
         case 'd':
            signum = true;
            base = 10;
            break;

         case 'u':
            base = 10;
            break;

         case 'o':
            base = 8;
            break;

      #ifndef CONFIG_NO_BINARY_PRINTF_FORMAT
         /*
          * CAUTION! Binary output by format %b isn't a part of ANSI-C!
          * ... But it simplifies the software developing. ;-)
          */
         case 'b':
            base = 2;
            /*
             * Unfortunately the padding size is one decimal digit only.
             * That isn't enough for binary output, which has a maximum of
             * 32 characters.
             * Therefore in the case of binary output the padding size
             * becomes multiplicated by 4.
             *
             * Suppressing compiler warnings about %b put CFLAGS += -Wno-format
             * in your makefile.
             */
            paddingWidth *= 4;
            break;
      #endif /* ifndef CONFIG_NO_BINARY_PRINTF_FORMAT */

         case 'x':
            base = 16;
            hexOffset = 'a' - '9' - 1;
            break;

         case 'p':
            if( paddingWidth == 0 )
            { /*
               * Set default format for pointer.
               */
               paddingWidth = sizeof( void* ) * 2;
               paddingChar = '0';
            }
            /* No break here! */
         case 'X':
            base = 16;
            hexOffset = 'A' - '9' - 1;
            break;

         case 'c':
            currentChar = va_arg( ap, int );
            /* No break here! */
         default:
            __PUT_CHAR( currentChar );
            continue;
      }

   #ifdef CONFIG_ENABLE_PRINTF64
      uint64_t u_val;
   #else
      uint32_t u_val;
   #endif
      u_val = va_arg( ap, typeof( u_val ) );
      if( signum && ((u_val & (1LL << (BIT_SIZEOF(u_val)-1))) != 0) )
      {
         __PUT_CHAR('-');
         u_val = -u_val;
      }

      unsigned char digitBuffer[BIT_SIZEOF(u_val)+1];
      ptr = digitBuffer + ARRAY_SIZE(digitBuffer);
      *--ptr = '\0';

      do
      {
         char ch = (u_val % base) + '0';
         if( ch > '9' )
            ch += hexOffset;

         *--ptr = ch;
         u_val /= base;

         if( paddingWidth != 0 )
            paddingWidth--;
      }
      while( u_val > 0 );

      while( paddingWidth-- != 0 )
         *--ptr = paddingChar;

      while( *ptr != '\0' )
         __PUT_CHAR( *ptr++ );
   } /* end while( true ) */
   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see mprintf.h
 */
int vprintf( const char* format, va_list ap )
{
   PRINTF_T printfObj =
   {
      .pStart   = NULL,
      .pCurrent = NULL,
      .limit    = 0,
      .putch    = sendToUart
   };
   return vprintfBase( &printfObj, format, ap );
}

/*! ---------------------------------------------------------------------------
 * @see mprintf.h
 */
int vsnprintf( char* s, size_t n, const char* format, va_list arg )
{
   PRINTF_T printfObj =
   {
      .pStart   = s,
      .pCurrent = s,
      .limit    = n,
      .putch    = addToString
   };
   return vprintfBase( &printfObj, format, arg );
}

/*! ---------------------------------------------------------------------------
 * @see mprintf.h
 */
int mprintf( const char* format, ... )
{
   int rval;
   va_list ap;
   va_start( ap, format );
   rval = vprintf( format, ap );
   va_end( ap );
   return rval;
}

#ifndef CONFIG_USE_LINUX_PRINTF
/*! ---------------------------------------------------------------------------
 * @see pp-printf.h
 */
int pp_printf( const char* format, ... )
{
   va_list ap;
   va_start( ap, format );
   const int rval = vprintf( format, ap );
   va_end( ap );
   return rval;
}

/*! ---------------------------------------------------------------------------
 * @see pp-printf.h
 */
int pp_sprintf( char* s, char const *format, ... )
{
   va_list ap;
   va_start( ap, format );
   const int r = vsnprintf( s, DEFAULT_SPRINTF_LIMIT, format, ap );
   va_end( ap );
   return r;
}

/*! ---------------------------------------------------------------------------
 * @see pp-printf.h
 */
int pp_vsprintf( char* buf, const char* format, va_list arg )
{
   PRINTF_T printfObj =
   {
      .pStart   = buf,
      .pCurrent = buf,
      .limit    = DEFAULT_SPRINTF_LIMIT,
      .putch    = addToString
   };
   return vprintfBase( &printfObj, format, arg );
}
#endif /* ifndef CONFIG_USE_LINUX_PRINTF */

/*! ---------------------------------------------------------------------------
 * @see mprintf.h
 */
int sprintf( char* s, char const *format, ... )
{
   va_list ap;
   va_start( ap, format );
   const int r = vsnprintf( s, DEFAULT_SPRINTF_LIMIT, format, ap );
   va_end( ap );
   return r;
}

/*! ---------------------------------------------------------------------------
 * @see mprintf.h
 */
int snprintf( char* s, size_t n, const char* format, ... )
{
   va_list ap;
   va_start( ap, format );
   const int r = vsnprintf( s, n, format, ap );
   va_end( ap );
   return r;
}

/*! ---------------------------------------------------------------------------
 * @deprecated Use macros in eb_console_helper.h instead.
 */
void m_cprintf( int color, const char *fmt, ... )
{
   va_list ap;
   mprintf( "\033[0%d;3%dm", color & C_DIM ? 2:1, color & 0x7f);
   va_start( ap, fmt );
   vprintf( fmt, ap );
   va_end( ap );
}

/*! ---------------------------------------------------------------------------
 * @deprecated Use macros in eb_console_helper.h instead.
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
 * @deprecated Use macros in eb_console_helper.h instead.
 */
void m_term_clear( void )
{
   mprintf("\033[2J\033[1;1H");
}

/*================================== EOF ====================================*/
