/***************************************************************************
 *   Copyright (C) 2008 by root   *
 *   ray@etheira.net   *
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

#include "oribotics.h"

FILE *logfile;	// file pointer for error logging
FILE *temp;      	// file for debugging info

int logging;
int verbose;

char logfilename[256];
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
	j=strftime(timeString,32,"%Y-%m-%d %H:%M:%S ",localtime(&now));       // local
	//
	// build the message to write to the log file
    	strcpy(logString, timeString);
	strcat(logString,  msgString);
	strcat(logString, "\n");

	return logString;
}
	
void logs ( char msgString[0x80]  )
/*
	Write a message into the log file and time stamp it.
	prepend the time and date to the string we are writing to the log file
	This file is to hold any error messages, or events of significance.
	 	
*/	
{
	if (logging==0) { 

		if (verbose==1)
		{
			printf("%s\n", msgString);
		}
		return; 
	}
   //
	logfile = fopen(logfilename,"a");
	fprintf (logfile, addtimeString(msgString) );
	fclose  (logfile);

	if (verbose==1)
	{
		printf("%s\n", msgString);
	}

}

