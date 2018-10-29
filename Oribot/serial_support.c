/***************************************************************************
*   Copyright (C) 2008 by Ray Gardiner                                    *
*   ray@etheira.net                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
/*
   	originally robin.c-- from the Linux Applications Development Book
	modified for use a a tool to talk to FB3's etc and illustrate the
	usage of the dspserial.h functions.
   	implements simple serial port interaction program
   	modified to use as a configurable minicom replacement for simple serial
   	tasks. Used here to provide hardware level access to serial ports

	This latest incarnation is part of The Border Project for audiencea
	interaction using hand held zigbee devices. 
*/

#include "serial_support.h"
#include "logging.h"

char   *portname;              /* device to use for serial IO ie /dev/ttyS1 */
char   *baudrate;              /* baud rate ie B9600 */

static struct termios pots;    /* old port termios settings to restore */
static struct termios sots;    /* old stdout/in termios settings to restore */
static struct termios pts;     /* termios settings on port */
static struct termios sts;     /* termios settings on stdout/in */
static struct sigaction sact;  /* used to initialize the signal handler */

static int    pf;              /* port file descriptor */
fd_set ready;                  /* used for select */

static struct timeval TimeOut; /* select timeout */

void RestoreConsoleTermios ( void ) 
{
	tcsetattr(STDIN_FILENO, TCSANOW, &sots); // restore console settings
}

void RestoreSerialTermios ( void ) 
{
	tcsetattr(pf, TCSANOW, &pots); // restore console settings
}


/* restore original terminal settings on exit */
void cleanup_termios(int signal) {
	tcsetattr(pf, TCSANOW, &pots);
	tcsetattr(STDIN_FILENO, TCSANOW, &sots);
	exit(0);
}

///////////////////////////////////////////////////////////////////
// non blocking Console IO functions

#define CTRLCHAR(ch) ((ch)&0x1f)

void InitConsole ( void )
{
/* setup console so that keys are immediately sent to serial port 
  as they are typed, rather than being buffered 
*/
	tcgetattr(STDIN_FILENO, &sts);
	sots = sts;
	/* again, some arbitrary things */
	sts.c_iflag &= ~(BRKINT | ICRNL);
	sts.c_iflag |= IGNBRK;
	sts.c_lflag &= ~ISIG;
	sts.c_cc[VMIN] = 1;
	sts.c_cc[VTIME] = 0;
	sts.c_cc[VERASE] =CTRLCHAR('H');
	sts.c_lflag &= ~ICANON;
	/* no local echo: allow the other end to do the echoing */
	sts.c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
	/* set the signal handler to restore the old
	* termios handler */
	sact.sa_handler = cleanup_termios;
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGINT, &sact, NULL);
	sigaction(SIGPIPE, &sact, NULL);
	sigaction(SIGTERM, &sact, NULL);
	
	/* Now set the modified termios settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &sts);
	
}

int iskeypressed ( void )
/* return true if keyboard has char waiting */
{
	TimeOut.tv_sec=0;
	TimeOut.tv_usec=500;
	
	FD_ZERO(&ready);
	FD_SET(STDIN_FILENO, &ready);
	FD_SET(pf, &ready);
	select(pf+1, &ready, NULL, NULL, &TimeOut );
	return (FD_ISSET(STDIN_FILENO, &ready));
}

unsigned char key ( void )
/*  return keboard keypress */
{   int res; unsigned char c;

	res = read(STDIN_FILENO, &c, 1);
	if (res<0) {
		exit(0);
	}
	return c;
}



void xemit ( unsigned char c )
{
	write(STDOUT_FILENO, &c, 1);
}


////////////////////////////////////////////////////////////////////
// portable serial IO functions

speed_t  symbolic_speed(char * baudrate)
{
	if (strcmp(baudrate,"B115200")==0) return B115200;
	if (strcmp(baudrate,"B57600")==0)  return B57600;
	if (strcmp(baudrate,"B38400")==0)  return B38400;
	if (strcmp(baudrate,"B19200")==0)  return B19200;
	if (strcmp(baudrate,"B9600" )==0)  return B9600;
	if (strcmp(baudrate,"B4800" )==0)  return B4800;
	if (strcmp(baudrate,"B2400" )==0)  return B2400;
	if (strcmp(baudrate,"B1200" )==0)  return B1200;
	if (strcmp(baudrate,"B600"  )==0)  return B600;
	if (strcmp(baudrate,"B300"  )==0)  return B300;
	if (strcmp(baudrate,"B150"  )==0)  return B150;
	if (strcmp(baudrate,"B134"  )==0)  return B134;
	if (strcmp(baudrate,"B110"  )==0)  return B110;
	if (strcmp(baudrate,"B75"   )==0)  return B75;
	if (strcmp(baudrate,"B0"   )==0)   return B0;
	// default to good ole 9600 8N1
	return B9600; 
}

static int cks;
static int rtsbits;
static int dtrbits;

void InitComms ( void )
{
	portname="/dev/ttyUSB0";
	//baudrate="B115200";
	baudrate="B9600";
	//baudrate="B19200";

	char s[0x100];
	sprintf(s,"STATUS : Opening Comms port %s at %s",portname,baudrate);
	logs(s);

	pf = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);

	if (pf < 0) {
		sprintf(s,"ERROR  : opening Comms Port %s at %s",portname,baudrate );
		logs(s);
		exit(0);
	}
	fcntl(pf, F_SETFL, FNDELAY);	// Set no-wait delay when buffer empty
	tcgetattr(pf, &pts);		 
	 	
	// input control c_iflag
	pts.c_iflag =  IGNBRK;		
	pts.c_iflag &= ~(IXON|IXOFF|IXANY);	

	// local control c_lflag
	pts.c_lflag = 0;

	// output control c_oflag			
 	pts.c_oflag = 0;	

	// control bits c_cflag		
	pts.c_cflag |= CLOCAL | CREAD;	

	pts.c_cflag &= ~(PARENB | PARODD);
	pts.c_cflag &= ~CSTOPB;
	pts.c_cflag &= ~CSIZE;	// zero 
	pts.c_cflag |= CS8;	// 8 bits/byte
	pts.c_cflag |= CRTSCTS; // enable hw flow control

	pts.c_cc[VMIN]  = 1;			
	pts.c_cc[VTIME] = 5;		

	cfsetospeed(&pts, symbolic_speed(baudrate));
	cfsetispeed(&pts, symbolic_speed(baudrate));

	//cfsetospeed(&pts, B9600);
	//cfsetispeed(&pts, B9600);

									
	tcsetattr(pf, TCSANOW, &pts);

	rtsbits = TIOCM_RTS;        		   		 		    	 
	ioctl(pf, TIOCMBIS, &rtsbits);    

	dtrbits = TIOCM_DTR;        		   		 		    	 
	ioctl(pf, TIOCMBIS, &dtrbits); 
  	

	sprintf(s,"SUCCESS: Comms port %s Opened at %s",portname,baudrate);
	logs(s);
}



int isrxready ( int timer)
{
	int res;
	TimeOut.tv_usec=timer;
	TimeOut.tv_sec=0;	
	FD_ZERO(&ready);
	FD_SET(pf, &ready);               // exit on rx ready
	res=select(pf+1, &ready, NULL, NULL, &TimeOut );
	return (FD_ISSET(pf, &ready));
}

unsigned char Rx  ( void )
{   
	char s[0x100];
	int res; unsigned char c;
	res = read(pf, &c, 1);
	if (res<0) {
		sprintf(s,"ERROR: read returned an error.. :-( %d\n",res);
		logs(s);
		RestoreConsoleTermios();
		RestoreSerialTermios(); 
		exit(0);
	}
	return c;
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
	ioctl(pf, TIOCSERGETLSR, &lsr);  // lsr = 1 when complete
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
	ioctl(pf, TIOCSERGETLSR, &lsr);  // lsr = 1 when complete
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
	ioctl(pf, TIOCMBIC, &rtsbits);	 
										
}


void SetTransmitMode()
/* 	Drive RTS line high to enable RS485 transmit
*/
{
    ioctl(pf, TIOCMBIS, &rtsbits);   	// set RTS
    ioctl(pf, TIOCMBIS, &dtrbits);   	// set DTR
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


void TL(unsigned char c)
/*	  transmit the character to the comms port, it appears
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
	write(pf, &c, 1);


 	//printf(" %x", c);

}

void Tx(unsigned char c)
/*	  transmit the character to the comms port, it appears
	  to require a short delay before transmitting?
	  This needs further investigation, by waiting for
	  shift complete we acheive our desired character timing!
*/
{
 // int i;
	SetTransmitMode();
      	AddtoCRC(c);
      	write(pf, &c, 1);
}

void small_delay ( void ) 
{
	int j;
	for (j=0;j<50000;j++) { 
		
	}
}

unsigned int RxPos[100];

unsigned int bot_enabled[71] = {
//  1  2  3  4  5  6  7  8  9 10
0,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 1-10
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 11-20
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 21-30
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 31-40
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 41-50
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 51-60
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1   // 61-70
};


int bot_cmd ( unsigned char cmd, unsigned char node, unsigned char data1,unsigned char data2 ) 
{
	unsigned char c;
	unsigned int RxNode;
	unsigned int pos;

	unsigned int flush;
	RxNode=0;
	RxPos[node]=100;  // default to no response

	c = (cmd<<6) | node;

	 
	if (isrxready(1000)) { flush = Rx();  } else { goto SendCommand; }
	/*
	if (isrxready(2000)) { flush = Rx();  } else { goto SendCommand; }
	if (isrxready(2000)) { flush = Rx();  } else { goto SendCommand; }
	if (isrxready(2000)) { flush = Rx();  } else { goto SendCommand; }
	if (isrxready(2000)) { flush = Rx();  } else { goto SendCommand; }
	if (isrxready(2000)) { flush = Rx();  } else { goto SendCommand; }
	*/

SendCommand:	

	Tx(0xa5); // flag char to alert oribot... 
	//small_delay();
	Tx(c);
	//small_delay();
	Tx(data1);
	Tx(data2);
	Tx(0xff); // flag char end packet

	SetReceiveMode();
	
 	
	if (node==63) {  // broadcast address no response expected 
		printf("Broadcast"); return 1;
	} 

	if (isrxready(20000)) { RxNode = Rx() & 0x3f;  } else { goto timeout1; }
	if (isrxready(20000)) { pos = Rx(); 
				   if (pos<100) { RxPos[node] =  pos; }
			       } else { goto timeout2; }
	//
	if (node==RxNode) {
		
		return 1;
	}
	return 0;

timeout1: 	 return 0;
timeout2: 	 return 0;
}



void OribotCommand ( unsigned char cmd, unsigned char node, unsigned char data1, unsigned char data2 ) 
{
	int retries;
	int tries;
	int error;
	char q[0x100];
	char s[0x100];

	error=0;
	retries=10;
	tries=0;
	sprintf(q,"Requesting Command %1d for Node %02d",cmd, node );

	if (bot_enabled[node]==0) {
		sprintf(s,"%s..Disabled",q);
		logs(s);
		RxPos[node]=100;
		return;
	}
	while ((error==0) && (retries>0)) {
		error = bot_cmd( cmd,node,data1,data2); 
		tries++;
		if (error==0) { retries--; usleep(20000); }
	}
	if (retries==0)  {
		sprintf(s,"%s..Timeout!",q); 
		update_bot_timedout(node, 0); // command to set a value in the table for true or false for timeout
		logs(s);
	}
	else {         
		sprintf(s,"%s RxPos[%02d]=%03d Tries=%02d",q, node, RxPos[node],tries );   
		update_bot_timedout(node, tries);
		logs(s);
	}
}

void serial_terminal ( int add_linefeed, int hexdump, int echo ) 
{
/* this implements a simple serial terminal, whatever is typed on the keyboard
   will be sent to the serial port, and anything chars received by the serial 
   port will be diaplayed on the console

   REQUIRES prior initialization of console and serial ports
   some flags modify things, ie add_linefeed, and hex dump and echo

   Exit when control A is typed

*/
	unsigned char c;
	char s[0x100];
	int done;
	done=0;
	xemit(0x2a);
	xemit(0x2a);
	xemit(0x2a);

	SetTransmitMode();

	do {
		while (isrxready(5000)) {        // if serial port char waiting
			
			c = Rx();            // read serial port
			if (add_linefeed) {
				if (c==0x0d) { xemit(0x0a); }
			}
			
			if (hexdump)  {
				// if we are dumping api mode packets detect demiliter 
				// and display on a new line  
				//if ((c==0x7e)||(c==0x88)||(c==0x89)) { xemit(0x0a); xemit(0x0d); }
				if ((c==0x7e)||(c==0x88)) { xemit(0x0a); xemit(0x0d); }
				//xemit (c&0x7f);
				sprintf(s,"%02x ",c&0xff );
				write(STDOUT_FILENO, s, strlen(s));
			}
			else {
				xemit(c);
			}
		}
	
		if ( iskeypressed()) {       	// if keyboard is pressed
			
			c = key();           	// read the key
			if (c==1 ) {            // exit if its control A key
				done=1;
				xemit(0x0a);
				xemit(0x0d);
				return;
			}

			if (echo) { 
				xemit(c);  // send to console if echoing  
			}	
			if (add_linefeed) {
				if (c==0x0d) { xemit(0x0a); } 
			}
			Tx(c);    // send to serial port

	
		}
	} while (!done);
	xemit(0x0a);
	xemit(0x0d);

}


