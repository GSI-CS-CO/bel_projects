#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "uart.h"

int vprintf(char const *format,va_list ap)
{
  unsigned char  	scratch[16];
  unsigned char  	format_flag;
  unsigned int 	u_val=0;
  unsigned char 	base;
  unsigned char 	*ptr;
  unsigned char 	width = 0;
  unsigned char  	fill;

  while(1)
  {
  
  	width = 0;
  	fill = ' ';
    while ((format_flag = *format++) != '%')
	{  
      if (!format_flag)
	  {
		  va_end (ap); 
		  return (0);
	  }
	
      uart_write_byte(format_flag);
    }


	// check for zero pad
	format_flag = *format - '0'; 
	if (format_flag == 0)	// zero pad
	{
		fill = '0';
		format++;
	}
	
	// check for width spec
	format_flag = *format - '0'; 
	if (format_flag > 0 && format_flag <= 9)	// width set
	{
		width = format_flag;
		format++;	
	}
	
    switch (format_flag = *format++)
	{
		case 'c':
		  format_flag = va_arg(ap,int);

		  //fall through
		  
		default:
      uart_write_byte(format_flag);

		  continue;
		  
		case 'S':
		case 's':
		  ptr = (unsigned char *)va_arg(ap, char *);
		  while (*ptr)
	          uart_write_byte(*ptr++);
		  continue;



		case 'd':

		  base = 10;
		  goto CONVERSION_LOOP;

		case 'u':
		  base = 10;
		  goto CONVERSION_LOOP;
		  
		case 'x':
		  base = 16;

CONVERSION_LOOP:

	  	u_val = va_arg(ap,unsigned int);
		if((format_flag=='d') && (u_val&0x80000000))
		{
		    uart_write_byte('-');
		    u_val=-u_val;
		}

		  	
		 ptr = scratch + 16;

		  *--ptr = 0;

		  do
		  {
			char ch = (u_val % base) + '0';
			if (ch > '9')
			  ch += 'a' - '9' - 1;

			*--ptr = ch;

			u_val /= base;

			if (width)
				width--;
		
		  } while (u_val>0);

//	  while (width--)
//		 	*--ptr = fill; 		      
      
		  while (*ptr)
	          uart_write_byte(*ptr++);

    }
  }
  return 0;
}


static int _p_vsprintf(char const *format,va_list ap, char*dst)
{
  unsigned char  	scratch[16];
  unsigned char  	format_flag;
  unsigned int 	u_val=0;
  unsigned char 	base;
  unsigned char 	*ptr;
  unsigned char 	width = 0;
  unsigned char  	fill;

  while(1)
  {
  
  	width = 0;
  	fill = ' ';
    while ((format_flag = *format++) != '%')
	{  
      if (!format_flag)
	  {
		  va_end (ap); 
		    *dst++=0;
		  return (0);
	  }
	
     *dst++=format_flag;
    }


	// check for zero pad
	format_flag = *format - '0'; 
	if (format_flag == 0)	// zero pad
	{
		fill = '0';
		format++;
	}
	
	// check for width spec
	format_flag = *format - '0'; 
	if (format_flag > 0 && format_flag <= 9)	// width set
	{
		width = format_flag;
		format++;	
	}
	
    switch (format_flag = *format++)
	{
		case 'c':
		  format_flag = va_arg(ap,int);

		  //fall through
		  
		default:
       *dst++=format_flag;

		  continue;
		  
		case 'S':
		case 's':
		  ptr = (unsigned char *)va_arg(ap, char *);
		  while (*ptr)
	           *dst++=*ptr++;
		  continue;



		case 'd':
		case 'u':
		  base = 10;
		  goto CONVERSION_LOOP;
		  
		case 'x':
		  base = 16;

CONVERSION_LOOP:

	  	u_val = va_arg(ap,unsigned int);
		  	
		 ptr = scratch + 16;

		  *--ptr = 0;

		  do
		  {
			char ch = (u_val % base) + '0';
			if (ch > '9')
			  ch += 'a' - '9' - 1;

			*--ptr = ch;

			u_val /= base;

			if (width)
				width--;
		
		  } while (u_val>0);

//	  while (width--)
//		 	*--ptr = fill; 		      
      
		  while (*ptr)
	           *dst++=*ptr++;

    }
  }
    *dst++=0;
  return 0;
}

int mprintf(char const *format, ...)
{
	int rval;
  	va_list ap;
  	va_start (ap, format);
  	rval = vprintf(format,ap);
  	va_end(ap);
  	return rval;
  	
}

int sprintf(char *dst, char const *format, ...)
{
  	va_list ap;
  	va_start (ap, format);
  	int r= _p_vsprintf(format,ap,dst);
		return r;
	
}

#define C_DIM 0x80                                                                                                    
void m_cprintf(int color, const char *fmt, ...)                                                                    
{
  va_list ap;                                                                                                       
  mprintf("\033[0%d;3%dm",color & C_DIM ? 2:1, color&0x7f);                                                          
  va_start(ap, fmt);
  vprintf(fmt, ap);                                                                                                 
  va_end(ap);
}

void m_pcprintf(int row, int col, int color, const char *fmt, ...)                                                 
{
  va_list ap;                                                                                                       
  mprintf("\033[%d;%df", row, col);
  mprintf("\033[0%d;3%dm",color & C_DIM ? 2:1, color&0x7f); 
  va_start(ap, fmt);                                                                                                
  vprintf(fmt, ap);                                                                                                 
  va_end(ap);
}

void m_term_clear()                                                                                                     
{
  mprintf("\033[2J\033[1;1H");                                                                                       
}

