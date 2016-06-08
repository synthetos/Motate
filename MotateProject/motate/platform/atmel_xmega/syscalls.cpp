/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
  * \file syscalls.c
  *
  * Implementation of newlib syscall.
  *
  */
#if 0
/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "syscalls.h"


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>


/*----------------------------------------------------------------------------
 *        Exported variables
 *----------------------------------------------------------------------------*/

#undef errno
extern int errno ;
extern int end ;

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
extern void _exit( int status ) ;
extern void _kill( int pid, int sig ) ;
extern int _getpid ( void ) ;


extern int link( char *cOld, char *cNew )
{
    return -1 ;
}

extern int _close( int file )
{
    return -1 ;
}

extern int _lseek( int file, int ptr, int dir )
{
    return 0 ;
}

extern int _read(int file, char *ptr, int len)
{
    return 0 ;
}

extern int _write( int file, char *ptr, int len )
{
//	size_t written = SerialUSB.write((const uint8_t *)ptr, len);
//	spi.write((const uint8_t *)ptr, len);
//	return written;

	return 0;

/*
    int iIndex ;
//    for ( ; *ptr != 0 ; ptr++ )
    for ( iIndex=0 ; iIndex < len ; iIndex++, ptr++ )
    {
//        UART_PutChar( *ptr ) ;

		// Check if the transmitter is ready
		  while ((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);

		  // Send character
		  UART->UART_THR = *ptr;
    }
    return iIndex ;
*/
}

extern void _exit( int status )
{
    printf( "Exiting with status %d.\n", status ) ;

    for ( ; ; ) ;
}

extern void _kill( int pid, int sig )
{
    return ;
}

extern int _getpid ( void )
{
    return -1 ;
}

#ifdef __cplusplus
}
#endif

#endif