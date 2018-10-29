/***************************************************************************
                          modbus.c  -  description
                             -------------------
    begin                : Thu Jan 17 2002
    copyright            : (C) 2002 by Ray Gardiner
    email                : ray@dsp.com.au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
    Linux Modbus Driver, RS-485 serial driver					
    Ray Gardiner &  Paul Arch,DSP systems 14/10/99 								
 					
*/

#ifndef __MODBUS_PROTOCOL
#define __MODBUS_PROTOCOL

#include "main.h"
#include "modbus.h"


//	
// uncomment if we have no modbus network
// #define nomodbus
//

static int cks;							    // Initialise checksum
static int dtrbits = 0;					// Initialise DTR bit
static int rtsbits = 0;					// Initialise RTS bit	

int MBaddress; 	// address of node to poll

#define rxBuffSize 256

int rxbuff[rxBuffSize];		// An array for Received data
short int rxData[100];

int txPacketCount[256];         	// array for txpacket counts
int rxPacketCount[256];           // array for rxpacket counts
int ErrorCount   [256];           // array for error counts
int TimeOutCount [256];           // array for timeout errors

char s[0x800];						// general purpose string buffer
char es[0x800];
int error=0;						// checking and recording

struct options;						// Structure for Port Settings
int serialport;

FILE *logfile;		// file pointer for error logging
FILE *temp;      	// file for debugging info

char logString[0x100];

char * addtimeString ( char msgString[0x100] )
{
	char timeString[0x80];  // 1999 Oct 22 23:59:59 GMT
	int j;
	//struct tm gmt;
	time_t now;        // needed for time()
    time_t tp;         //
	now = time(&tp);   // copy current time into *now
	//
	//j=strftime(timeString,32,"\n%Y %b %d %H:%M:%S %Z ",gmtime(&now));     // zulu
	j=strftime(timeString,32,"\n%Y-%m-%d %H:%M:%S ",localtime(&now));    // local
	//
	// build the message to write to the log file
    strcpy(logString, timeString);
	strcat(logString,  msgString);

	return logString;
}
	
void logprint ( char msgString[0x80]  )
/*
	Write a message into the log file and time stamp it.
	prepend the time and date to the string we are writing to the log file
	This file is to hold any error messages, or events of significance.
	 	
*/	
{
	if (logging==0) { return; }
   //
	logfile = fopen(logfilename,"a");
	fprintf (logfile, addtimeString(msgString) );
	fclose  (logfile);
	if (verbose==1)
	{

			printf("%s", addtimeString(msgString));
	}

}





int TxShifting ( void )
/*	The uart buffers data in the shift register, and  although
	the transmit register may be empty, the last char to be sent
	is still being shifted out, so we delay changing to receive
	until the last bit has been shifted out, and lsr = 1	
	I think TIOCSERGETLSR returns the whole LSR register so
	we mask out the other bits to ensure we ONLY look at the
	shift completed bit
*/
{
	int lsr;
	ioctl(serialport, TIOCSERGETLSR, &lsr);  // lsr = 1 when complete
	return (  !(lsr & 0x01) );       // lsr = 0 when still shifting
}


int TxDataRegisterFull ( void )
/*	The uart buffers data in the shift register, and  although
	the transmit register may be empty, the last char to be sent
	is still being shifted out, so we delay changing to receive
	until the last bit has been shifted out, and lsr = 1	
	I think TIOCSERGETLSR returns the whole LSR register so
	we mask out the other bits to ensure we ONLY look at the
	shift completed bit
*/
{
	int lsr;
	ioctl(serialport, TIOCSERGETLSR, &lsr);  // lsr = 1 when complete
	return (  !(lsr & 0x02) );       // lsr = 0 when still shifting
}

void SetReceiveMode()				
/* 	This function sets the Serial port in receive mode by toggling the
   	RTS line. Is mainly used after we send data, as it checks the LSR first
        before it sets back to receive to make sure all  data has been sent
	(line shift register empty)
*/										 	
{	

	while ( TxShifting() )  { continue; }
	while ( TxShifting() )  { continue; }
	while ( TxShifting() )  { continue; }	
	ioctl(serialport, TIOCMBIC, &rtsbits);	 	// clear RTS										
}


void SetTransmitMode()
/* 	Drive RTS line high to enable RS485 transmit
*/
{	ioctl(serialport, TIOCMBIS, &rtsbits);   	// set RTS
}

void AddtoCRC( int c)
/*  	This code is adapted from the PDL Serial comms book
	it's the basic shift and XOR polynomial CRC
*/
{
	int i;

	for(i=0; i<8; i++)
	{
	  if(((c^cks) & 0x0001)==1) {
		cks =((cks>>1)^0xA001);
	  }
	  else {
		cks = cks>>1;
	  }
	c = c >> 1;
	}
}	


void Tx(unsigned char c)
/*	transmit the character to the comms port, it appears
	  to require a short delay before transmitting?
	  This needs further investigation, by waiting for
	  shift complete we acheive our desired character timing!
*/
{	
 // int i;
  AddtoCRC(c);
	while ( TxShifting() )  { continue; }
	while ( TxShifting() )  { continue; }
	while ( TxShifting() )  { continue; }	
	write(serialport, &c, 1);


 	//printf(" %x", c);

}

//////////////////////////////////////////////////////////////////////////////////
    #define mbdigital  5       // modbus address for digital inputs
		#define Port 0x180

    #define mbanalog   4
		#define ch1  0x210
		#define ch2  0x211
		#define ch3  0x212
		#define ch4  0x213


void RHR_request(unsigned int Addr, unsigned int regs )
/* 	Transmit a Read Holding Register request from Addr
	Need to set MBaddress elsewhere
*/
{
	unsigned char h;			//
	unsigned char l;			//
	cks=0xFFFF;					  // Initialise the Checksum
                                            //
	SetTransmitMode();			// Set the transmit MODE
	Tx(MBaddress);	  			// Transmit the Modbus Address
	Tx(0x03);					      // 0x03 == Read Holding Register Function Type..
	                            //
	Tx ( (unsigned char)(Addr >> 8) );		  // Form lowbyte/hibyte of
	Tx ( (unsigned char)(Addr & 0x00ff) );	// the memory location
                                					//
	Tx ( (unsigned char)(regs >> 8  ) );	  // number of registers to read
	Tx ( (unsigned char)(regs & 0x00ff) );
                                          //
	h=(unsigned char)(cks >> 8);						//  Form lowbyte/hibyte of
	l=(unsigned char)(cks & 0x00FF);				// the checksum and send it..
	Tx(l);  				
	Tx(h);  				
                                //
	txPacketCount[MBaddress]++; //
	SetReceiveMode();			// Set back to receive
    ClearRXBuffer();
    //printf("Sent RHR request...\n");
	
}

//////////////////////////////////////////////////////////////////////////////////

int Rx( int timer )
/*	This receives a character using "select" to wait for
	either a char or timeout,
*/
{
	int res;
	struct timeval Timeout;	
	fd_set readfs;						
	int FDnumber=0;						

	Timeout.tv_usec =timer;		   		//  wait
	Timeout.tv_sec  = 0;  		    	// for packet to start
	FDnumber=serialport+1;				  //
	FD_ZERO( &readfs);						  // clears &readfs
	FD_SET( serialport,&readfs);		// adds fd to the set
/*
	Now for the tricky bit, the following call to select will
	only return when one of the file descrptors is ready to be
	written. Note: this might not be the complete packet to
	cater for this possibility, in the next layer	
*/
	res= select (FDnumber, &readfs, NULL, NULL, &Timeout);
	return (res);
/*   	
	if select timed out res will be 0, if some data is waiting
	to be read then res will be non-zero.
*/						
}

//////////////////////////////////////////////////////////////////////////////////

int RxPacket ( int expected )
/*
	either returns with a complete packet, or times out
	wait up to 500ms for first char, if first char
	is recieved, then change timeout to 10ms to
	wait for end of packet..
*/
{
  int res, n, i;
  int len;
  int ratio;

	res = Rx(500000);
	if ( res )
	{
 		for (i=0; i<expected; i++)
        {   // read one char at a time into rxbuffer

		    res = Rx(20000);
		    n=read( serialport, &rxbuff[i], 1);
			if (n!=-1)
			{
		      //printf("%2x ",rxbuff[i]);
            }
			else
			{
                if (rxPacketCount[MBaddress]>0)
                {
                    ratio = TimeOutCount[MBaddress]*100/txPacketCount[MBaddress];
                }
                else 
                {
                    ratio = 100;
                }
                printf("\nModbus TimeOut Packet %3d %5d %5d %5d %5d%%",MBaddress, txPacketCount[MBaddress], rxPacketCount[MBaddress], TimeOutCount[MBaddress], ratio );
				goto TimedOut;
	        }
		}
		TimedOut:
		len=i;
		rxPacketCount[MBaddress]++;
       // printf("Len=%d Exp=%d\n",len, expected);
		return len;
	}
	else
	{	// don't log this as an error until it retries out!
		// since the remote sometimes is performing an un-interruptable
		// function, like writing to eeprom or reading serial adc data
		// we will get timeouts as a matter of course
		//
		 TimeOutCount[MBaddress]++; //
     if (rxPacketCount[MBaddress]>0)
     {
        ratio = TimeOutCount[MBaddress]*100/txPacketCount[MBaddress];
     }
     else 
     {
        ratio = 100;
     }
     //printf("\nModbus TimeOut Start  %3d %5d %5d %5d %5d%%",MBaddress, txPacketCount[MBaddress], rxPacketCount[MBaddress], TimeOutCount[MBaddress], ratio );

	  return 0;  // timed out waiting for 1st char
	}
}

//////////////////////////////////////////////////////////////////////////////////

void ClearRXBuffer()				
{						
	tcflush(serialport, TCIOFLUSH);			
}						


static int r;  // number of bytes received


#define NoErrors 	0
#define BadDataLength 	1
#define BadFunctionCode 2
#define BadAddress      3
#define BadCRC 		4
#define BadPacketLength 5
#define TimedOut 	6
#define Busy 		8
#define Nack            9
#define Unsupported    10
#define Unknown        11
#define IllegalData    12


int AnalyseRHR_Response( void )
/*
	Analyse response to RHR request and validate response
	before passing result back to calling routine with
	data (if any) in rxData. r contains the received bytecount
*/
{
	int i;
	// dump rx buffer to debug file for diagnostic purposes
	//for (i=0; i<r; i++) {  printf( "%3x ", receivedData[i] ); }
	//printf("\n");
	cks=0xffff;
	for (i=0; i<r; i++) { AddtoCRC( rxbuff[i] );  }
	if ( cks!=0)        { return BadCRC;                }
	//
	// correct checksum for packet received
	if ( MBaddress != rxbuff[0] ) { return BadAddress; }
	//
	// expected address received  	
    if (rxbuff[1] == 0x83)
	{
		// an standard error response of some sort
		// check the error code in the message
		if (rxbuff[2]==0x07) { return  Nack;        }
		if (rxbuff[2]==0x06) { return  Busy;        }
		if (rxbuff[2]==0x01) { return  Unsupported; }
		return Unknown;
	}

    if (rxbuff[1] != 0x03 ) { return BadFunctionCode; }
/*
	if we managed to get this far, chances are good that the data is ok
*/

	  rxData[0] = (rxbuff[3] << 8)        & 0xff00 ;   // hi
	  rxData[0] = rxData[0] + (rxbuff[4]  & 0x00ff);   // lo
		//rxData[0] = 1000;

	  rxData[1] = (rxbuff[5] << 8)        & 0xff00 ;   // hi
	  rxData[1] = rxData[1] + (rxbuff[6]  & 0x00ff);   // lo
		//rxData[1] = 1001;

	  rxData[2] = (rxbuff[7] << 8)        & 0xff00 ;   // hi
	  rxData[2] = rxData[2] + (rxbuff[8]  & 0x00ff);   // lo
		//rxData[2] = 1002;

	  rxData[3] = (rxbuff[9] << 8)        & 0xff00 ;   // hi
	  rxData[3] = rxData[3] + (rxbuff[10] & 0x00ff);   // lo
		//rxData[3] = 1003;

	  rxData[4] = (rxbuff[11] << 8)       & 0xff00 ;   // hi
	  rxData[4] = rxData[4] + (rxbuff[12] & 0x00ff);   // lo
		//rxData[4] = 1004;

	  rxData[5] = (rxbuff[13] << 8)       & 0xff00 ;   // hi
	  rxData[5] = rxData[5] + (rxbuff[14] & 0x00ff);   // lo
		//rxData[5] = 1005;

	  rxData[6] = (rxbuff[15] << 8)       & 0xff00 ;   // hi
	  rxData[6] = rxData[6] + (rxbuff[16] & 0x00ff);   // lo

      rxData[7] = (rxbuff[17] << 8)        & 0xff00 ;   // hi
	  rxData[7] = rxData[7] + (rxbuff[18] & 0x00ff);   // lo
		//rxData[3] = 1003;

	  rxData[8] = (rxbuff[19] << 8)       & 0xff00 ;   // hi
	  rxData[8] = rxData[8] + (rxbuff[20] & 0x00ff);   // lo


	  rxData[9] = (rxbuff[21] << 8)       & 0xff00 ;   // hi
	  rxData[9] = rxData[9] + (rxbuff[22] & 0x00ff);   // lo

	  rxData[10] = (rxbuff[23] << 8)       & 0xff00 ;   // hi
	  rxData[10] = rxData[10] + (rxbuff[24] & 0x00ff);   // lo

	  return 0;
}


int RHR (unsigned int NetAddr, unsigned int Addr, unsigned int regs)
{
	char msgString[0x100];
	int retries=3;
	int error  =6;     					// default to timeout error
	int k;

    MBaddress=NetAddr;
//	sprintf(msgString,"Read NetAddr %d MBaddr %d ", MBaddress, Addr+40001 );
//
//  if debugging without modbus network
//

	while ( (retries!=0) && (error!=0) )
	{

		RHR_request( Addr,regs );     	 // transmit the request packet
		r = RxPacket(5+(regs*2));          // wait for a response
		if ( r>=1 )
		{	
			error = AnalyseRHR_Response();
    	if  (error)
			{
				retries--;
				usleep(50000); // wait 50 ms before retring
			}
		}  	
		else
		{
			retries--;
			usleep(50000); // wait 50 ms before retring
		}
	}                 

	if 	(error)
	{
 		switch (error)
		{
		case NoErrors: 		strcat(msgString," Ok "); 				        break;
		case BadDataLength: strcat(msgString," Bad Data Length"); 	 	    break;
		case BadFunctionCode:strcat(msgString," Bad Function");   		    break;
		case BadAddress: 	strcat(msgString," Bad Address" );   		    break;
 		case BadCRC:		strcat(msgString," Bad CRC" );   		        break;
		case BadPacketLength:strcat(msgString," Bad Packet Length" );       break;
		case TimedOut:		strcat(msgString," Retry Count Exceeded" );     break;
		case Busy:			strcat(msgString," Busy" ); 				    break;
		case Nack:			strcat(msgString," Nack" ); 		            break;
		case Unsupported:	strcat(msgString," Unsupported Function" );     break;
		case IllegalData:   strcat(msgString," Illegal Data Range" );       break;
		default:			strcat(msgString," Unknown Error " );		    break;
		
		}	
		// write error into log file
		logprint(msgString);
	}
	else
	{
		if (verbose==1)
		{
				sprintf(s, " Data ");
				strcat(msgString,s);

			    for (k=0;k<regs;k++)
				{
					sprintf(s, "%5d ", rxData[k]);
					strcat(msgString,s);
				}
				strcat(msgString,"\n");
				printf(msgString);
		}
	}
	return error;
}


int AnalysePOLL_Response( void )
/*
	Analyse response to RHR request and validate response
	before passing result back to calling routine with
	data (if any) in rxData. r contains the received bytecount
*/
{
	int i;
	// dump rx buffer to debug file for diagnostic purposes
	//for (i=0; i<r; i++) {  printf( "%3x ", receivedData[i] ); }
	//printf("\n");
	cks=0xffff;
	for (i=0; i<r; i++) { AddtoCRC( rxbuff[i] );  }
	if ( cks!=0)        { return BadCRC;                }
	//
	// is itcorrect checksum for packet received
	if ( MBaddress != rxbuff[0] ) { return BadAddress; }
	//
	// expected address received  	
    if (rxbuff[1] == 0x83)
	{
		// an standard error response of some sort
		// check the error code in the message
		if (rxbuff[2]==0x07) { return  Nack;        }
		if (rxbuff[2]==0x06) { return  Busy;        }
		if (rxbuff[2]==0x01) { return  Unsupported; }
		return Unknown;
	}

    if (rxbuff[1] != 0x03 ) { return BadFunctionCode; }
/*
	if we managed to get this far, chances are good that the data is ok
*/

	  rxData[0] = (rxbuff[3] << 8)        & 0xff00 ;   // hi
	  rxData[0] = rxData[0] + (rxbuff[4]  & 0x00ff);   // lo
		//rxData[0] = 1000;

	  rxData[1] = (rxbuff[5] << 8)        & 0xff00 ;   // hi
	  rxData[1] = rxData[1] + (rxbuff[6]  & 0x00ff);   // lo
		//rxData[1] = 1001;

	  rxData[2] = (rxbuff[7] << 8)        & 0xff00 ;   // hi
	  rxData[2] = rxData[2] + (rxbuff[8]  & 0x00ff);   // lo
		//rxData[2] = 1002;

	  rxData[3] = (rxbuff[9] << 8)        & 0xff00 ;   // hi
	  rxData[3] = rxData[3] + (rxbuff[10] & 0x00ff);   // lo
		//rxData[3] = 1003;

	  rxData[4] = (rxbuff[11] << 8)       & 0xff00 ;   // hi
	  rxData[4] = rxData[4] + (rxbuff[12] & 0x00ff);   // lo
		//rxData[4] = 1004;

	  rxData[5] = (rxbuff[13] << 8)       & 0xff00 ;   // hi
	  rxData[5] = rxData[5] + (rxbuff[14] & 0x00ff);   // lo
      
	  rxData[6] = (rxbuff[15] << 8)       & 0xff00 ;   // hi
	  rxData[6] = rxData[6] + (rxbuff[16] & 0x00ff);   // lo

      rxData[7] = (rxbuff[17] << 8)        & 0xff00 ;   // hi
	  rxData[7] = rxData[7] + (rxbuff[18] & 0x00ff);   // lo
		//rxData[3] = 1003;

	  rxData[8] = (rxbuff[19] << 8)       & 0xff00 ;   // hi
	  rxData[8] = rxData[8] + (rxbuff[20] & 0x00ff);   // lo


	  rxData[9] = (rxbuff[21] << 8)       & 0xff00 ;   // hi
	  rxData[9] = rxData[9] + (rxbuff[22] & 0x00ff);   // lo
   
	  rxData[10] = (rxbuff[23] << 8)       & 0xff00 ;   // hi
	  rxData[10] = rxData[10] + (rxbuff[24] & 0x00ff);   // lo


	  return 0;
}

int POLL (unsigned int NetAddr, unsigned int Addr, unsigned int regs)
{
	char msgString[0x100];


    MBaddress=NetAddr;
	sprintf(msgString,"Polling For Device %d MBaddr %d ", MBaddress, Addr+40001 );

	RHR_request( Addr,regs );     	 // transmit the request packet
	r = RxPacket(5+(regs*2));          // wait for a response

	error = AnalysePOLL_Response();

    return error;
}


int AnalyseWHR_ReceivedPacket( void )
/*
	Analyse response to WHR request and validate response
	before passing result back to calling routine
*/
{   int i;
	//dump rx buffer to debug file for diagnostic purposes
	//for (i=0; i<r; i++) {  printf( "%3x ", receivedData[i] ); }
	//printf("\n");

	cks=0xffff;
	for (i=0; i<r; i++) { AddtoCRC( rxbuff[i] );  }
	if ( cks!=0)        { return BadCRC;                }
	//
	// correct checksum for packet received
	if ( MBaddress != rxbuff[0] ) { return BadAddress; }
	//
	// expected address received  	
    if (rxbuff[1] == 0x86)
	{
		// an standard error response of some sort
		// check the error code in the message
		if (rxbuff[2]==0x07) { return  Nack;        }
		if (rxbuff[2]==0x06) { return  Busy;        }
		if (rxbuff[2]==0x01) { return  Unsupported; }
		if (rxbuff[2]==0x03) { return  IllegalData; }
		return Unknown;
	}

    if (rxbuff[1] != 0x06 ) { return BadFunctionCode; }
/*
	if we managed to get this far, chances are good that the data is ok
*/
	return 0;
}




int WHR ( unsigned int NetAddr, unsigned int Addr, unsigned int Data)
{
	char msgString[0x100];
	unsigned char h;			//
	unsigned char l;			//
	int retries=20;
	int error  =6;     			// default to no error

	MBaddress=NetAddr;
	//sprintf(msgString,"WriteNetAddr %d MBaddr %d Data %d  \n", MBaddress, Addr+40001, Data);

	while ( (retries!=0) && (error!=0) )	
    {
		cks=0xFFFF;								// Initialise the Checksum
                                				//
		SetTransmitMode();						// Set the transmit MODE
		Tx(MBaddress);	  						// Transmit the Modbus Address
		Tx(0x06);								// 0x06 == Write Holding Register Function Type..
	                            				//
		Tx ( (unsigned char)(Addr >> 8) );		// Form lowbyte/hibyte of
		Tx ( (unsigned char)(Addr & 0x00FF) );	// the memory location
                                				//
		Tx ( (unsigned char)(Data >> 8) );      //
		Tx ( (unsigned char)(Data & 0x00FF) );	//
                                				//
		h=(  unsigned char )(cks >> 8);			//  Form lowbyte/hibyte of
		l=(  unsigned char )(cks & 0x00FF);		// the checksum and send it..
		Tx(l);  								//
		Tx(h);  								//
 		SetReceiveMode();						// Set back to receive
        ClearRXBuffer();
		txPacketCount[MBaddress]++; 			//
		r = RxPacket(8);          				// wait for a response
        printf("\n");

		if (r>=1)
		{
			error = AnalyseWHR_ReceivedPacket();
			if (error)
			{  retries--;
			   usleep(10000);
			}
		}
		else
		{	 strcat(msgString,".");
		     retries--;
		     usleep(10000);
		}
	}

	// write something to the log file to record the error
 	switch (error)
	{
		case NoErrors: 			     strcat(msgString," Ok "); 				       break;
		case BadDataLength: 	     strcat(msgString," Bad Data Length"); 	 	   break;
		case BadFunctionCode:	     strcat(msgString," Bad Function");   		   break;
		case BadAddress: 		     strcat(msgString," Bad Address" );   		   break;
 		case BadCRC:			     strcat(msgString," Bad CRC" );   			   break;
		case BadPacketLength:	     strcat(msgString," Bad Packet Length" );      break;
		case TimedOut:			     strcat(msgString," Retry Count Exceeded" );   break;
		case Busy:				     strcat(msgString," Busy" ); 				   break;
		case Nack:				     strcat(msgString," Nack" ); 				   break;
		case Unsupported:		     strcat(msgString," Unsupported Function" );   break;
		case IllegalData:            strcat(msgString," Illegal Data Range" );     break;
		default:				     strcat(msgString," Unknown Error " );		   break;

	}
    if (error)
    {

				logprint(msgString);
    }

	return error;
	
}

int AnalyseWMHR_ReceivedPacket( void )
/*
	Analyse response to WHR request and validate response
	before passing result back to calling routine
*/
{   int i;
	//dump rx buffer to debug file for diagnostic purposes
	//for (i=0; i<r; i++) {  printf( "%3x ", receivedData[i] ); }
	//printf("\n");

	cks=0xffff;
	for (i=0; i<r; i++) { AddtoCRC( rxbuff[i] );  }
	if ( cks!=0)        { return BadCRC;                }
	//
	// correct checksum for packet received
	if ( MBaddress != rxbuff[0] ) { return BadAddress; }
	//
	// expected address received  	
    if (rxbuff[1] == 0x86)
	{
		// an standard error response of some sort
		// check the error code in the message
		if (rxbuff[2]==0x07) { return  Nack;        }
		if (rxbuff[2]==0x06) { return  Busy;        }
		if (rxbuff[2]==0x01) { return  Unsupported; }
		return Unknown;
	}

    if (rxbuff[1] != 0x10 ) { return BadFunctionCode; }
/*
	if we managed to get this far, chances are good that the data is ok
*/
	return 0;
}



int WMHR ( unsigned int NetAddr, unsigned int Addr, unsigned int Data)
{
  char msgString[0x100];
	unsigned char h;			//
	unsigned char l;			//
	int retries=20;
	int error  =6;     			// default to no error

	MBaddress=NetAddr;
	while ( (retries!=0) && (error!=0) )	
    {
		sprintf(msgString,"WriteMNetAddr %d MBaddr %d Data %d", MBaddress, Addr+40001, Data);

		cks=0xFFFF;								// Initialise the Checksum
                                				//
		SetTransmitMode();						// Set the transmit MODE
		Tx(MBaddress);	  						// Transmit the Modbus Address
		Tx(0x10);								// 0x06 == Write Holding Register Function Type..
	                            				//
		Tx ( (unsigned char)(Addr >> 8) );		// Form lowbyte/hibyte of
		Tx ( (unsigned char)(Addr & 0x00FF) );	// the memory location

		Tx ( 0x00);                             // number of registers
	    Tx ( 0x01);                             // always 1

		Tx ( 0x02);                             // number of data bytes follwoing
                                				//
		Tx ( (unsigned char)(Data >> 8) );      //
		Tx ( (unsigned char)(Data & 0x00FF) );	//
                                				//
		h=(  unsigned char )(cks >> 8);		    //  Form lowbyte/hibyte of
		l=(  unsigned char )(cks & 0x00FF);		// the checksum and send it..
		Tx(l);  								//
		Tx(h);  								//
 		SetReceiveMode();						// Set back to receive
        ClearRXBuffer();
		txPacketCount[MBaddress]++; 			//
		r = RxPacket(11);          				// wait for a response

		if (r>=1)
		{
			error = AnalyseWMHR_ReceivedPacket();
			if (error)
			{  retries--;
			   usleep(10000);
			}
		}
		else
		{	 strcat(msgString,".");
		     retries--;
		     usleep(10000);
		}
	}

	// write something to the log file to record the error
 	switch (error)
	{
		case NoErrors: 			   strcat(msgString," \tOk "); 				   break;
		case BadDataLength: 	   strcat(msgString," Bad Data Length"); 	   break;
		case BadFunctionCode:	   strcat(msgString," Bad Function");   	   break;
		case BadAddress: 		   strcat(msgString," Bad Address" );   	   break;
 		case BadCRC:			   strcat(msgString," Bad CRC" );   		   break;
		case BadPacketLength:	   strcat(msgString," Bad Packet Length" );    break;
		case TimedOut:			   strcat(msgString," Retry Count Exceeded" ); break;
		case Busy:				   strcat(msgString," Busy" ); 				   break;
		case Nack:				   strcat(msgString," Nack" ); 				   break;
		case Unsupported:		   strcat(msgString," Unsupported Function" ); break;
		case IllegalData:          strcat(msgString," Illegal Data Range" );   break;
		default:				   strcat(msgString," Unknown Error " );	   break;

	}
    if (error)
		{
				logprint(msgString);
		}

	return error;
	
}


int AnalyseFMC_ReceivedPacket( void )
/*
	Analyse response to FMC request and validate response
	before passing result back to calling routine
*/
{   int i;
	//dump rx buffer to debug file for diagnostic purposes
	for (i=0; i<r; i++) {  printf( "%3x ", rxbuff[i] ); }
	printf("\n");

	cks=0xffff;
	for (i=0; i<r; i++) { AddtoCRC( rxbuff[i] );  }
	if ( cks!=0)        { return BadCRC;          }
	//
	// correct checksum for packet received
	if ( MBaddress != rxbuff[0] ) { return BadAddress; }
	//
	// expected address received
    if (rxbuff[1] == 0x86)
	{
		// an standard error response of some sort
		// check the error code in the message
		if (rxbuff[2]==0x07) { return  Nack;        }
		if (rxbuff[2]==0x06) { return  Busy;        }
		if (rxbuff[2]==0x01) { return  Unsupported; }
		return Unknown;
	}

    if (rxbuff[1] != 0x0f ) { return BadFunctionCode; }
/*
	if we managed to get this far, chances are good that the data is ok
*/
	return 0;
}



// force multiple coils
int FMC32 ( unsigned int NetAddr, unsigned int Addr, unsigned int Data1, unsigned int Data2)
{
  char msgString[0x100];
	unsigned char h;			//
	unsigned char l;			//
	int retries=20;
	int error  =6;     			// default to no error
    int rc;
    //rc = pthread_mutex_lock(&mb_lock);
    //if (rc) {
    //  logprint("Modbus Mutex lock Error");
    //  pthread_exit(NULL);
    //}
	MBaddress=NetAddr;
	while ( (retries!=0) && (error!=0) )
    {
		sprintf(msgString,"Force Mult Coils Net=%d Caddr %d Data1 %x Data2 %x", MBaddress, Addr+1, Data1,Data2);

		cks=0xFFFF;								// Initialise the Checksum
                                				//
		SetTransmitMode();						// Set the transmit MODE
		Tx(MBaddress);	  						// Transmit the Modbus Address
		Tx(0x0f);								// 0x0f == force multiple coils..
	                            				//
		Tx ( (unsigned char)(Addr >> 8) );		// Form lowbyte/hibyte of
		Tx ( (unsigned char)(Addr & 0x00FF) );	// the memory location

		Tx ( 0x00);                             //
		Tx ( 0x20);                             // always 32 bits

        Tx ( 0x04);                             // number of data bytes

        Tx ( (unsigned char)(Data1 & 0x00FF) );	////
		Tx ( (unsigned char)(Data1 >> 8) );     //


        Tx ( (unsigned char)(Data2 & 0x00FF) );	 //
		Tx ( (unsigned char)(Data2 >> 8) );      //

                                				 //
		h=(  unsigned char )(cks >> 8);		     //  Form lowbyte/hibyte of
		l=(  unsigned char )(cks & 0x00FF);		 // the checksum and send it..
		Tx(l);  								 //
		Tx(h);  								 //
 		SetReceiveMode();						// Set back to receive
        ClearRXBuffer();
		txPacketCount[MBaddress]++; 			//
		r = RxPacket(8);          				// wait for a response

		if (r>=1)
		{
			error = AnalyseFMC_ReceivedPacket();
			if (error)
			{  retries--;
			   usleep(10000);
			}
		}
		else
		{	 strcat(msgString,".");
		     retries--;
		     usleep(10000);
		}
	}

	// write something to the log file to record the error
 	switch (error)
	{
		case NoErrors: 			   strcat(msgString," \tOk "); 				   break;
		case BadDataLength: 	   strcat(msgString," Bad Data Length"); 	   break;
		case BadFunctionCode:	   strcat(msgString," Bad Function");   	   break;
		case BadAddress: 		   strcat(msgString," Bad Address" );   	   break;
 		case BadCRC:			   strcat(msgString," Bad CRC" );   		   break;
		case BadPacketLength:	   strcat(msgString," Bad Packet Length" );    break;
		case TimedOut:			   strcat(msgString," Retry Count Exceeded" ); break;
		case Busy:				   strcat(msgString," Busy" ); 				   break;
		case Nack:				   strcat(msgString," Nack" ); 				   break;
		case Unsupported:		   strcat(msgString," Unsupported Function" ); break;
		case IllegalData:          strcat(msgString," Illegal Data Range" );   break;
		default:				   strcat(msgString," Unknown Error " );	   break;

	}
    if (error)
		{
				logprint(msgString);
                printf(msgString);
		}
        //	  dprint (msgString);
        //  rc=pthread_mutex_unlock(&mb_lock);
        //  if (rc) {
        //     logprint("Modbus mutex unlock Error");
        // }
  return error;

}



void InitializeCommPort()
{
/*	Initialize commport modbusDEV using POSIX style termios
	structures

*/
	struct termios options;				// Set up structure for serial settings 	
										// and then open up the FD(serial port ttyS1)

	serialport = open(modbusDEV, O_RDWR | O_NOCTTY | O_NDELAY);
	fcntl(serialport, F_SETFL, FNDELAY);	
										// Set no-wait delay when buffer empty
										// 	
	tcgetattr(serialport, &options);	// get FD settings
	 	
	cfsetispeed(&options, B9600);		// Set input speed  to 9600bps
	cfsetospeed(&options, B9600);		// Set output speed to 9600bps 	 	
	options.c_iflag =  IGNBRK;			        //
	options.c_iflag &= ~(IXON|IXOFF|IXANY);	
	options.c_lflag = 0;				//
 	options.c_oflag = 0;				//
	options.c_cflag |= CLOCAL | CREAD;	//
	options.c_cflag &= ~CRTSCTS;		//

////
// Sets parity EVEN
//	options.c_cflag |= PARENB;			//
//	options.c_cflag &= ~PARODD;			//
//	options.c_cflag &= ~CSTOPB;			//
//	options.c_cflag &= ~CSIZE;			//
//	options.c_cflag |= CS8;				//
//	options.c_cflag &= ~CRTSCTS;		//


////
// Sets parity NONE
//
	options.c_cflag &= ~(PARENB | PARODD);	//
	options.c_cflag &= ~CRTSCTS;			//


	options.c_cc[VMIN] = 1;				//
	options.c_cc[VTIME] = 5;			//
										// Store settings in
	tcsetattr(serialport, TCSANOW, &options);	// structure...
	rtsbits = TIOCM_RTS;        		   // Set ioctl's variables for	 	
	dtrbits = TIOCM_DTR;			    // DTR and RTS bits...
	ioctl(serialport, TIOCMBIS, &dtrbits);      // apply power to RS485 module via DTR
                                                // Keep applied to supply power for 		 			
	                							// RS-232 to RS-485 convertor
	SetReceiveMode();							// Set in initial receive mode

	logprint("Modbus Comms Initialized\n");
}

void CloseComms()
{
 	logprint("Modbus Comms Closed\n");
	close(serialport);
}



#endif




















































































































































































































