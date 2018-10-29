/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Wed Jun 19 11:48:51 EST 2002
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "main.h"
#include "modbus.h"

int logging;
int verbose;
int logging;


char ipaddress[256];
char logfilename[256];

char * version = " 1.5 October 2007";

void bin_prt ( int x)
{
/*
	 print x as binary ie. 1000 1101 1111 0011
*/
 	int n;
	for (n=0;n<16;n++)
	{
   		if ( (x&0x8000)!=0 )                  { printf("1"); }
			else                              { printf("0"); }
			if ((n==3)||(n==7)||(n==11))      { printf(" "); }
			x = x<<1;
	}
	printf(" ");
}

void Pulses ( int Node )
{
		int i;
 		if (RHR(Node,527,1)==0)
		{
				printf("\nNode=%2d ", Node );

                //printf("Digital IO=");
                //bin_prt(rxData[0]);

                RHR(Node,880,8);

				for (i=0;i<8;i++) {	
					printf("%4d ", rxData[i]);
				}	

                RHR(Node,888,8);

				for (i=0;i<8;i++) {	
					printf("%4dms ", rxData[i]);
				}	

                return;
		}
        printf("\n Node %2d is not responding", Node);
}

void Identify ( int Node )
{
        int id;
        int vsn;
 		if (RHR(Node,320,2)==0)
		{
            vsn=rxData[0];
            id=rxData[1];
			printf("\nNode=%2d ", Node );
            switch (id) {
            case 11: printf("Satlink over radio Version=%d",        vsn); break;
            case 12: printf("Standard Satlink Version=%d",          vsn); break;
            case 13: printf("Standard Aquator Unimod Version=%d",   vsn); break;
            case 14: printf("Unimod Counter Version=%d",            vsn); break;
            case 15: printf("Modem enabled Alarm UniMod Version=%d",vsn); break;
            case 50: printf("Standard Super Unimod Version=%d",     vsn); break;
            case 51: printf("Pump Controller Unimod Version=%d",    vsn); break;
            case 100:printf("Sawmill Controller  Version=%d",       vsn); break;
            case 101:printf("Sawmill Controller  Version=%d",       vsn); break;
            case 102:printf("Sawmill Cylinder Controller =%d",      vsn); break;

            default: printf("Unknown unimod type Version=%d",       vsn); break;
            }
            return;
		}
        else {
            if (  RHR(Node,3300,1)==0) {
                printf("\nNode=%2d ABB Drive Found ACS400 Software Version %04x", Node, rxData[0] );
            }
            else {
                printf("\nNode=%2d - - - - - ", Node);
            }
        }
}


void Poll ( int Node )
{
		int i;
 		if (RHR(Node,1,1)==0)
		{
				printf("\nNode=%2d ", Node );

                printf("Digital IO=");
                bin_prt(rxData[0]);

                if (RHR(Node,26,4)==0)
				for (i=0;i<4;i++) {	
					printf("A%1d=%4d, ",i,rxData[i]);
				}	
                return;
		}
        printf("\n - - - - Node %2d is not responding", Node);
}

void DHR ( int Node, int Addr, int regs )
{
		int i;
 		if (RHR(Node,Addr,regs)==0)
		{
				printf("\nNode=%2d ", Node );

                //printf("Digital IO=");
                //bin_prt(rxData[0]);

				for (i=0;i<regs;i++) {	
					printf("%3d=%4d, ",Addr+i,(unsigned int)(rxData[i]));
				}
                return;	
		}
        printf("\n - - - - Node %2d is not responding", Node);
}

int Read ( int node, int addr)
{
    if (RHR(node,addr,1)==0) {
        return (rxData[0]);
    }
    return 0;
}

void DumpConfig ( int addr)
{
    Identify( addr);
    if (1)
    {
    //printf("\n 273  BaudRate = %d", Read(addr,273));
    //printf("\n 304  Address = %d", Read(addr,304));
    //printf("\n 833  Edges per Inch = %d", Read(addr,833));

    //}
    //else
    //{
    printf("\n 273  BaudRate = %d", Read(addr,273));
    printf("\n 276  FailSafe Time = %d", Read(addr,276));
    printf("\n 277  PulsePerMinute Interval = %d", Read(addr,277));
    printf("\n 334 PWM Scaling = %d", Read(addr,334));
    printf("\n 335 PWM Offset  = %d", Read(addr,335));
    printf("\n 336 Analog In Calibration Constant Channel 1=%d", Read(addr,336));
    printf("\n 337 Analog In Calibration Constant Channel 2=%d", Read(addr,337));
    printf("\n 338 Analog In Calibration Constant Channel 3=%d", Read(addr,338));
    printf("\n 339 Analog In Calibration Constant Channel 4=%d", Read(addr,339));
    printf("\n 340 Smoothing Period = %d", Read(addr,340));
    printf("\n 341 Smoothing Index = %d", Read(addr,341));
    printf("\n 345 Pulse Input Polling Interval = %d mS", Read(addr,345));
    printf("\n 345 Pulse Input Edge Direction = %d (+ve=1, -ve=0)", Read(addr,346));
    printf("\n 352 Pulse Input 1 Prescaller = %d", Read(addr,352));
    printf("\n 353 Pulse Input 2 Prescaller = %d", Read(addr,353));
    printf("\n 354 Pulse Input 3 Prescaller = %d", Read(addr,354));
    printf("\n 355 Pulse Input 4 Prescaller = %d", Read(addr,355));
    printf("\n 356 Pulse Input 5 Prescaller = %d", Read(addr,356));
    printf("\n 357 Pulse Input 6 Prescaller = %d", Read(addr,357));
    printf("\n 358 Pulse Input 7 Prescaller = %d", Read(addr,358));
    printf("\n 359 Pulse Input 8 Prescaller = %d", Read(addr,359));

    printf("\n 432 Relay 1 TimeOut = %d", Read(addr, 432));
    printf("\n 433 Relay 2 TimeOut = %d", Read(addr, 433));
    printf("\n 434 Relay 3 TimeOut = %d", Read(addr, 434));
    printf("\n 435 Relay 4 TimeOut = %d", Read(addr, 435));
    printf("\n 436 Relay 5 TimeOut = %d", Read(addr, 436));
    printf("\n 437 Relay 6 TimeOut = %d", Read(addr, 437));
    printf("\n 438 Relay 7 TimeOut = %d", Read(addr, 438));
    printf("\n 439 Relay 8 TimeOut = %d", Read(addr, 439));

    }

}


void RelayOn ( int rly )
{                                                                    	
    WHR (5,416+rly,1);
	usleep(4000); 	
}

void RelayOff ( int rly )
{                                                                    	
    WHR (5,416+rly,0);
	usleep(4000);
}


MYSQL     *conn;
MYSQL_RES *res_set;
MYSQL_ROW row;


#define def_host_name "127.0.0.1"
#define def_user_name "root"
#define def_password  "cb2225a"
#define def_db_name   "MorfDB"

char msg[200];
//char  s[1024];

int mysql_failed;

pthread_mutex_t mysql_lock = PTHREAD_MUTEX_INITIALIZER;

void lock_db ( void )
{
    int rc;
    rc = pthread_mutex_lock(&mysql_lock);
    if (rc) {
         //logw("\nMySQL Mutex lock Error");
         pthread_exit(NULL);
    }
}

void unlock_db ( void )
{
    int rc;
    rc = pthread_mutex_unlock(&mysql_lock);
    if (rc) {
      //logw("\nMySQL Mutex unlock Error");
      pthread_exit(NULL);
    }
}

void ConnectToDataBase()
{
  conn = mysql_init (NULL);
  if (conn==NULL)
  {
   		printf("Error 1:Connecting to mysql\n");
			//
			// optionally start with default settings?
			//
			return;
	}
  if(mysql_real_connect (
			conn,
			def_host_name,
			//ipaddress,
			def_user_name,
			def_password,
			def_db_name,
            0,
			NULL,
			0)==NULL)
      {
   		   printf("Error 2:Connecting to mysql\n");
		   return;
      }

}


void DBupdate( char * q)
{

  char s[0x100];
  lock_db();
  mysql_failed=0;
  //logw( q );
  if(mysql_query(conn,q) !=0)
  {
     	sprintf(s,"Error 4:SQL Update Query  Failed: %s \n",q );
       // logw(s);
        mysql_failed=1;

        unlock_db();
        return;
   }
  unlock_db();

}


int dbres;
int num_rows;
int num_cols;

void DBread( char * q)
{
   lock_db();

  if(mysql_query(conn,q) !=0)
  {
     	printf("Error 3:SQL Select Query  Failed: %s \n",q );
        return;
  }
  res_set=mysql_store_result(conn);

  num_rows = mysql_num_rows(res_set);
  num_cols = mysql_num_fields(res_set);

  if (res_set==NULL) { dbres=0; return; }
  row=mysql_fetch_row(res_set);
  dbres=1;

}

    int update_ready;
    int Relay1;
    int Relay2;
    int Relay3;
    int Relay4;
    int RelaysOn;
    int OC1;
    int OC2;
    int OC3;
    int OC4;



void dbupdate ( int n )
{
    char q[0x200];
    
    ConnectToDataBase();
    
    //printf("Checking data base for ADCModule %d\n", n);

    sprintf(q,"select update_ready,Relay1,Relay2,Relay3,Relay4,OC1,OC2,OC3,OC4 from ADCModule where id=%d",n);
    DBread(q);
    if (dbres==1) {
        sscanf( row[0],"%d",&update_ready);
        
        //if( strcmp( row[1],"OFF")==0) { Relay1=0; } else { Relay1=1; }
        //if( strcmp( row[2],"OFF")==0) { Relay2=0; } else { Relay2=1; }
        //if( strcmp( row[3],"OFF")==0) { Relay3=0; } else { Relay3=1; }
        //if( strcmp( row[4],"OFF")==0) { Relay4=0; } else { Relay4=1; }
        
        sscanf( row[1],"%d",&Relay1);
        sscanf( row[2],"%d",&Relay2);
        sscanf( row[3],"%d",&Relay3);
        sscanf( row[4],"%d",&Relay4);
        sscanf( row[5],"%d",&OC1);
        sscanf( row[6],"%d",&OC2);
        sscanf( row[7],"%d",&OC3);
        sscanf( row[8],"%d",&OC4);
        
        unlock_db();
        
        RelaysOn = (Relay4<<3) + (Relay3<<2) + (Relay2<<1) + Relay1;
    
        
        if (update_ready==1) {

             RelaysOn = (Relay4<<3) + (Relay3<<2) + (Relay2<<1) + Relay1;
             
             WHR(n,38,RelaysOn);                                                    
             
             if (OC1!=0) {    // cycle power to OC1
                WHR(n,40,OC1);
                sprintf(q,"update ADCModule set OC1='0' where id='%d'",n);
                DBupdate (q);
             }
             if (OC2!=0) {   // cycle power to OC2
                WHR(n,41,OC2);
                sprintf(q,"update ADCModule set OC2='0' where id='%d'",n);
                DBupdate (q);
             }     
             if (OC3!=0) {    // cycle power to OC3
                WHR(n,42,OC3);
                sprintf(q,"update ADCModule set OC3='0' where id='%d'",n);
                DBupdate (q);
             }
             if (OC4!=0) {   // cycle power to OC4
                WHR(n,43,OC4);
                sprintf(q,"update ADCModule set OC4='0' where id='%d'",n);
                DBupdate (q);
             }
        }
    }
   
    sprintf(q,"update ADCModule set update_ready='0' where id='%d'",n);
    DBupdate (q);

}



void help ()
{
	printf ("\n Command line optons for mcu (Modbus Config Utility");
	printf ("\n ");
	printf ("\n BASIC MODBUS COMMANDS");
	printf ("\n  -R xx          ... Read from node xx ");
	printf ("\n  -W xx          ... Write to node xx  ");
    printf ("\n  -a aaaa        ... Address of register to read/write to");
    printf ("\n  -d dddd        ... Data to write");
	printf ("\n  -n nn          ... Number of registers to read");
    printf ("\n  -v             ... Show extra modbus debug information");
    printf ("\n  -i ww.xx.yy.zz X.. use modbus/tcp");
    printf ("\n  -l <logfile>   X.. file for transaction log");
    printf ("\n");
	printf ("\n EXAMPLE BASIC MODBUS COMMANDS");
	printf ("\n Read Holding register example:");
	printf ("\n mcu -R 12 -a 1234 -n 10     read from node 12 register 1234..for 10 registers");
	printf ("\n");
    printf ("\n Write Holding register example:");
	printf ("\n mcu -W 11 -a 1234 -d 8765   write 8765 to node 11 register 1234");
    printf ("\n");
    printf ("\n PUMP UNIMOD and SUPER UNIMOD specific commands");
    printf ("\n  -I aa          ... Identify Software type and Version at address aa'");
	printf ("\n  -S aa bb       ... Scan for and identify unimods from node aa to bb");
	printf ("\n  -D aa          ... Dump configuration data for unimod");
	printf ("\n  -C aa bb       ... Change address aa to address bb");
	printf ("\n  -F aa          ... Force node aa to default settings");
	printf ("\n  -L aa          ... Look at current IO status");
	printf ("\n  -1 aa nn       ... Turn on  relay 0..7 at address nn");
    printf ("\n  -0 aa nn       ... Turn off relay 0..7 at address nn");
	printf ("\n  -T aa          ... Continuous dump of analog and Digital values ");
    printf ("\n  -P aa          ... Continuous dump of pulse counters ");
    printf ("\n  -Z aa          ... Test Comms to ABB drive");
	printf ("\n ");
	printf ("\n OTHER COMMANDS");
	printf ("\n  -V             ... Print Version and exit");
	printf ("\n  -?             ... Print this help.");
    printf ("\n");

}

#define read  1
#define write 2
#define scan  3
#define tcpip 4

int operation;

int main(int argc, char *argv[])
{

    float T1,T2,T3,T4,T5,T6;
    float V1,V2;
    char q[0x200];
	int i,j;
    int function;
    int operation=0;
	int node_address = 0;
    int register_address =0;
    int data_to_write=0;
	int number_of_registers=0;
    int address_to_test=0;

    int node_valid    =0;
    int reg_valid     =0;
    int data_valid    =0;
    int num_reg_valid =0;
    int test_mode     =0;

    int start_node    =0;
    int end_node    =255;
    
    int from_address  =0;
    int to_address	  =0;

	verbose=0;
	logging=0;
	strcpy(ipaddress,"10.5.0.13");
	strcpy(logfilename,"/var/log/adc.log");
    //printf("\n....");
    function=0;

    InitializeCommPort();
    //monitor();
    //return 0;

	for (i=1; i<argc; i++)
	{
	 	if (argv[i]==NULL) { continue; }
		if ((argv[i][0]=='-') && (argv[i][2]=='\0'))
		{
            // single letter preceeded by '-'
			switch (argv[i][1])
			{
				case 'V': printf("\n MorfADC Configuration Utility (c) Ray Gardiner %s \n\n",version ); exit(0); break;
				case 'v': verbose=1;                                          break;
                case 'U': sscanf( argv[i+1], "%d", &node_address);                   i++; function     =11; break;
                case 'R': sscanf( argv[i+1], "%d", &node_address);  operation=read;  i++; node_valid   =1;  break;
				case 'W': sscanf( argv[i+1], "%d", &node_address);  operation=write; i++; node_valid   =1;  break;
				case 'a': sscanf( argv[i+1], "%d", &register_address);               i++; reg_valid    =1;  break;
				case 'd': sscanf( argv[i+1], "%d", &data_to_write);                  i++; data_valid   =1;  break;
				case 'n': sscanf( argv[i+1], "%d", &number_of_registers);   	     i++; num_reg_valid=1;  break;
                case 't': sscanf( argv[i+1], "%d", &address_to_test);                i++; test_mode    =1;  break;
                case 'S': sscanf( argv[i+1], "%d", &start_node);
                          sscanf( argv[i+2], "%d", &end_node);                 i++;  i++; function     =1;  break;
                case 'T': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =2;  break;
                case 'P': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =3;  break;
                case 'C': sscanf( argv[i+1], "%d", &from_address);
                          sscanf( argv[i+2], "%d", &to_address);               i++;  i++; function     =4;  break;
                case 'I': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =5;  break;
                case 'F': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =6;  break;
                case 'D': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =7;  break;
                case 'Z': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =8;  break;
                case 'x': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =9;  break;
                case 'y': sscanf( argv[i+1], "%d", &start_node);                     i++; function     =10; break;
				case 'i':
						if (argv[i+1]!=NULL)
						{
							strcpy(ipaddress,argv[i+1]);
							i++;
						}
						else
						{
							printf ("\nPlease supply ip address \n\n"); exit(0);	
						}
						break;
				case '?':
						help();
						exit(0);
						break;

				case 'l':
						if (argv[i+1]!=NULL)
						{
							strcpy(logfilename,argv[i+1]);
							i++;
							logging=1;
						}
						else
						{
							printf ("\nPlease a file name to log to");	
						}
						break;					
		    	default:
						printf ("\n Error: %s is not a valid option!\n\n", argv[i] );
						help();
						exit(0);
						break;
			}
		}
	}


     InitializeCommPort();

     if (function==1) {
        printf("\nScanning Network from %d to %d", start_node, end_node);
        for (i=start_node;i<end_node+1;i++) {
          Identify(i);
        }
        printf("\n");
        return 0;
     }
     if (function==2) {
        printf("\nMonitoring Node %d ", start_node);
        while (1) {
           Poll(start_node);
           usleep(100000);
        }
        printf("\n");
        return 0;
     }
     if (function==3) {
        printf("\nMonitoring Node %d ", start_node);
        while (1) {
           Pulses(start_node);
           usleep(100000);
        }
        printf("\n");
        return 0;
     }
     if (function==4) {
        printf("\nChanging Node %d Address to %d ", from_address, to_address );
        if (RHR(from_address,304,1)==0)  {
            printf("\n...Node %d is currently set to Address %d", from_address, rxData[0] );
        }
        else {
            printf("\n...Whoops, there is no node at address %d, what do I do now?", from_address);
            return 0;
        }
        printf("\n...ok, found node at address %d, changing address to %d", from_address, to_address);
        WHR(from_address,304,to_address);
        printf("\nNow Cycle device Power to re-read Node Address from eeprom");
        printf("\n");
        return 0;
     }

     if (function==5) {
        printf("\nAttempting to Identify Node %d ", start_node);
        Identify(start_node);
        printf("\n");
        return 0;
     }
     if (function==6) {
        printf("\nAttempting to Force Node %d to defaults", start_node);
        WHR(start_node,256,255);
        DumpConfig(start_node);
        printf("\n");
        return 0;
     }
     if (function==7) {

        DumpConfig(start_node);
        printf("\n");
        return 0;
     }
     if (function==8) {

        for (;;) {
            if (  RHR(start_node,3300,1)==0) {
                printf("\nABB Software Version %04x", rxData[0] );
            }
            else {
                printf("\n......ABB Drive did not respond");
            }
        }
        printf("\n");
        return 0;
     }

     if (function==9) {
        printf("Function 9\n");
        return 0;
     }

     if (function==10) {
        printf("Function 10\n");
        return 0;
     }
     if (function==11) {
         printf("Function 11 \n");
         for (j=1;j<4;j++) {
            dbupdate(j);
            if ( RHR(j,28,10)==0 ) {
                T1=(float)(rxData[0])/10;
                T2=(float)(rxData[1])/10;
                T3=(float)(rxData[2])/10;
                T4=(float)(rxData[3])/10; 
                T5=(float)(rxData[4])/10;
                T6=(float)(rxData[5])/10;
                V1=(float)(rxData[6])/100;
                V2=(float)(rxData[7])/100;

                printf("N=%d %d%d%d%d=%d %d%d%d%d ",j,Relay1,Relay2,Relay3,Relay4,RelaysOn,OC1,OC2,OC3,OC4);

                
                printf("t1=%4.1f, ",T1);  // 28
                printf("t2=%4.1f, ",T2);  // 29
                printf("t3=%4.1f, ",T3);  // 30
                printf("t4=%4.1f, ",T4);  // 31
                printf("t5=%4.1f, ",T5);  // 32               
                printf("t6=%4.1f, ",T6);  // 33
                printf("sv=%4.1f, ",V1);  // 34
                printf("bv=%4.1f, ",V2);  // 35
                printf("RD=%4d, ", rxData[8]);  // 36
                printf("RB=%4d, ", rxData[9]);  // 37

                sprintf(q,"update ADCModule set T1='%2.1f' where id='%d'",T1,j); DBupdate (q);
                sprintf(q,"update ADCModule set T2='%2.1f' where id='%d'",T2,j); DBupdate (q);
                sprintf(q,"update ADCModule set T3='%2.1f' where id='%d'",T3,j); DBupdate (q);
                sprintf(q,"update ADCModule set T4='%2.1f' where id='%d'",T4,j); DBupdate (q);
                sprintf(q,"update ADCModule set T5='%2.1f' where id='%d'",T5,j); DBupdate (q);
                sprintf(q,"update ADCModule set T6='%2.1f' where id='%d'",T6,j); DBupdate (q);
                sprintf(q,"update ADCModule set V1='%2.2f' where id='%d'",V1,j); DBupdate (q);
                sprintf(q,"update ADCModule set V2='%2.2f' where id='%d'",V2,j); DBupdate (q);

            	 
            }
            else {
                    printf("......morf ADC %d did not respond",j);
                    sprintf(q,"update ADCModule set T1='%2.1f' where id='%d'",0.0,j); DBupdate (q);
                    sprintf(q,"update ADCModule set T2='%2.1f' where id='%d'",0.0,j); DBupdate (q);
                    sprintf(q,"update ADCModule set T3='%2.1f' where id='%d'",0.0,j); DBupdate (q);
                    sprintf(q,"update ADCModule set T4='%2.1f' where id='%d'",0.0,j); DBupdate (q);
                    sprintf(q,"update ADCModule set T5='%2.1f' where id='%d'",0.0,j); DBupdate (q);
                    sprintf(q,"update ADCModule set T6='%2.1f' where id='%d'",0.0,j); DBupdate (q);
                    sprintf(q,"update ADCModule set V1='%2.2f' where id='%d'",0.0,j); DBupdate (q);
                    sprintf(q,"update ADCModule set V2='%2.2f' where id='%d'",0.0,j); DBupdate (q);

            }
            printf("\n");
        }
        printf("\n\n");
        return 0;
     }


     if (test_mode==1) {  
       printf("\nTesting Node %d, press ^C to exit \n ", address_to_test );
       for (;;) {
            DHR( address_to_test, 320, 1);
       }
 	 }
	 if (operation==read)
	 {
      	//printf ("Reading from Node=%d Reg=%d NumReg=%d", node_address, register_address, number_of_registers);
       if  (node_valid    ==0) { printf("\n...ERROR, node address not specified\n");         exit(0); }
       if  (reg_valid     ==0) { printf("\n...ERROR, register address not specified\n");     exit(0); }
       if  (num_reg_valid ==0) { printf("\n...ERROR, number of registers not specified\n");  exit(0); }

        for (;;) {
		    DHR(node_address, register_address, number_of_registers);
            usleep(200000);
       }
     }
	 if (operation==write)
	 {
       if  (node_valid   ==0)  { printf("\n...ERROR, node address not specified\n");         exit(0); }
       if  (reg_valid    ==0)  { printf("\n...ERROR, register address not specified\n");     exit(0); }
       if  (data_valid   ==0)  { printf("\n...ERROR, data to write not specified\n");        exit(0); }

      	printf ("Writing to Node=%d Reg=%d Data=%d", node_address, register_address, data_to_write);
        WHR(node_address,register_address, data_to_write);
     }

     //for (;;)
     //{
     //   n = rand()%256;
	 //	WHR(5,424,n);
	 //	for (i=0;i<8;i++) {  RelayOff(i); 			
     //}


   CloseComms();
   printf("\n");
   return EXIT_SUCCESS;
}
