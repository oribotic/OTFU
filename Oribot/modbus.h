/***************************************************************************
                          modbus.h  -  description
                             -------------------
    begin                : Thu Oct 21 1999
    copyright            : (C) 1999 by Ray Gardiner  & Paul Arch
    email                : ray@dsp.com.au, paul@dsp.com.au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef __MODBUS_H
#define __MODBUS_H

#define Debugging  1 				// uncomment if debugging info is desired

#define modbusDEV "/dev/ttyAMA0"		// Commport for modbus network

//#define RHRfunction 0x03			// Modbus ReadHoldingRegister  function
//#define WHRfunction 0x06			// Modbus WriteHoldingRegister function
//#define RHRsubtract 40001			// Subtract 40001 for Modbus ReadHoldingReg

extern int  MBaddress; 	
extern void logprint ( char * );
extern void EventLog ( char *, char * );
extern int  WHR  ( unsigned int, unsigned int , unsigned int );
extern int  WMHR ( unsigned int, unsigned int , unsigned int );

extern int  RHR ( unsigned int, unsigned int, unsigned int );
extern int  FMC32 ( unsigned int,  unsigned int, unsigned int, unsigned int);
extern void ClearRXBuffer();
extern void InitializeCommPort();
extern void CloseComms();
extern char * addtimeString ( char * );
extern short int  rxData[100];

//extern float ReadPressure();
//extern float ReadSuction();
//extern float ReadFlow();
//extern float ReadVoltage();
//extern int   ReadDigitalIO();
//extern void  WriteDigitalIO(int);
//extern void  ReadModbusInputs();
//extern void  UpdateModbusInputs();
//extern float Read420Flow();
//extern void  monitor();

#endif














