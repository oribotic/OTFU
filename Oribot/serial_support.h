/***************************************************************************
                          serial_support.h	
                             -------------------
    begin                :  Thu 17th January 2008
    copyright            : (C) 2008 by Ray Gardiner
    email                : ray@etheira.net
 **************************************************************************/
/**************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __serial_support_h
#define __serial_support_h

#include "oribotics.h"  
#include "serial_support.c"  

extern void serial_terminal ( int , int, int);
extern void Tx ( unsigned char );
extern unsigned char Rx  ( void );
extern int isrxready ( int );
extern void xemit ( unsigned char );
extern void InitConsole ( void );
extern void InitComms ( void );
extern char *portname;  /* device to use for serial IO ie /dev/ttyS1 */
extern char *baudrate;  /* baud rate ie B9600 */
extern int pf;
extern void OribotCommand (unsigned char, unsigned char, unsigned char, unsigned char);

extern void RestoreConsoleTermios ( void );
extern void RestoreSerialTermios ( void );
extern unsigned int bot_enabled[]; 
extern unsigned int RxPos[];

#endif
