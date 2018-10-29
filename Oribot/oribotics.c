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

#include "oribotics.h"
#include "serial_support.h"
#include "logging.h"


char * version = "TS7800 AEC Oribotics 2010 Network Control September 1st 2010 /dev/ttyAMAO ";

#define read  1
#define write 2
#define scan  3
#define tcpip 4

MYSQL     *conn;
MYSQL_RES *res_set;
MYSQL_ROW row;


#define def_host_name "127.0.0.1"
#define def_user_name "oribot"
#define def_password  "oribot"
#define def_db_name   "oribotics"

char msg[200];
char  s[1024];
int number_of_bots = 51;

int mysql_failed;

pthread_mutex_t mysql_lock = PTHREAD_MUTEX_INITIALIZER;

void lock_db ( void )
{
    int rc;
    rc = pthread_mutex_lock(&mysql_lock);
    if (rc) {
         logs("\nMySQL Mutex lock Error");
         pthread_exit(NULL);
    }
}

void unlock_db ( void )
{
    int rc;
    rc = pthread_mutex_unlock(&mysql_lock);
    if (rc) {
      logs("\nMySQL Mutex unlock Error");
      pthread_exit(NULL);
    }
}

int ConnectToDataBase()
{
    logs("ConnectToDataBase()");
    conn = mysql_init(NULL);
    if (conn==NULL)
    {
      logs("Error 1:Connecting to mysql");
      //
      // optionally start with default settings?
      //
      return 0;
    }
    if(mysql_real_connect (conn, def_host_name, def_user_name, def_password, def_db_name, 0, NULL,0)==NULL)
    {
      logs("Error 2:Connecting to mysql");
      return 0;
    }
    logs("Success connecting to DataBase");
    return 1;
}


void DBupdate( char * q)
{

  char s[0x100];
  lock_db();
  mysql_failed=0;

  if(mysql_query(conn,q) !=0)
  {
     	sprintf(s,"Error 4:SQL Update Query  Failed: %s",q );
        logs(s);
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
  char s[0x100];
  lock_db();

  //logs(q);

  if(mysql_query(conn,q) !=0)
  {
     	sprintf(s,"Error 3:SQL Select Query  Failed: %s",q );
		logs(s);
        unlock_db();
        return;
  }
  res_set=mysql_store_result(conn);

  num_rows = mysql_num_rows(res_set);
  num_cols = mysql_num_fields(res_set);

  if (res_set==NULL) { 
	mysql_free_result(res_set);
	dbres=0; 
	return; 
  }
  //row=mysql_fetch_row(res_set);
  //logs("mysql query ok");
  dbres=1;

}

int operation;
int terminal_mode;
int add_linefeed;
int hexdump;
int echo;



unsigned int bot_updated[] = {
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0
};

unsigned int pattern_step[]= {
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0
};

unsigned int physical_address[] = {
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0
};         

// scan order is used to randomise the scanning function

unsigned int scan_order[] = { 0,
	 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
	 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
	 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
	 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
	 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
	 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
};


int patternID;
int pattern_num_steps;
int logical;
int logical_target;
int physical;
int enabled;
int speed;
int position;
int sequence;
int step;

int get_pattern_id ( int bot)
{
	char query[0x100];
	sprintf(query,"select patternID from bot where logical=%d", bot);
	DBread(query);
	
	if (dbres) {
		row=mysql_fetch_row(res_set);
		sscanf( row[0], "%d", &patternID  );
	}
	else {
		patternID=1;  // default
	}
    unlock_db();
	mysql_free_result(res_set);
	return patternID;
}
 
int get_pattern_num_steps ( int bot)
{
	char query[0x100];
	sprintf(query,"select count(id) from pattern_item where patternID=(select patternID from bot where logical=%d)", bot);
	DBread(query);
	
	if (dbres) {
		row=mysql_fetch_row(res_set);
		sscanf( row[0], "%d", &pattern_num_steps  );
	}
	else {
		pattern_num_steps=10;  // default
	}
    unlock_db();
	mysql_free_result(res_set);
	return pattern_num_steps;
}
       

void get_pattern_details( int patternID, int step)
{
	char query[0x100];

	sprintf(query,"select logical,speed,position,sequence from pattern_item where patternID=%d and sequence=%d", patternID,step);
	DBread(query);
	
	if (dbres) {
		row=mysql_fetch_row(res_set);
		sscanf( row[0], "%d", &logical_target  );
		sscanf( row[1], "%d", &speed  );
		sscanf( row[2], "%d", &position  );
		sscanf( row[3], "%d", &sequence  );
	}
	else {
		logical_target=1;
		speed=20;
		position=50;
		sequence=1;
	}
    unlock_db();
	mysql_free_result(res_set);

}     

void reset_bot_enables ( void ) 
{                                  
	/* set all bots to enabled */
	// set the internal register to enabled for all bots
	// later they will be reset by the database, but in the process add a value for timeout or ok
	char query[0x100];               
	int k;  
	
	sprintf(s,"pre database reset");  
	logs(s); 
	unlock_db();             	    
	sprintf(query,"update bot set timedout='0'");
	DBupdate(query);
	unlock_db(); 

	sprintf(s,"completed reset in database");
	logs(s);
	
	for (k=1;k<number_of_bots;k++) {
		bot_enabled[k] = 1;       // enable all bots
	}   
}

void update_bot_enables ( ) 
{
	char query[0x100];
	int k;
	for (k=1;k<number_of_bots;k++) {
		sprintf(query,"select enabled,physical from bot where logical=%d",k);
		DBread(query);
		
		if (dbres) {
			row=mysql_fetch_row(res_set);
			sscanf( row[0], "%d", &bot_enabled[k] );
			sscanf( row[1], "%d", &physical_address[k] );
		}
		else {
			bot_enabled[k]=1;
			physical_address[k]=1;
		}
    	unlock_db();
		mysql_free_result(res_set);
	}
}         

void update_bot_timedout (unsigned char node, unsigned char trycount)
{     
	char query[0x100];                   
	// sets the timedout value for a bot to 1 or 0;
	sprintf(query,"update bot set timedout='%d' where logical=%d",trycount, node); 
	//sprintf(s,"update bot set timedout='%d' where logical=%d",trycount, node); 
	//logs(s);
	DBupdate(query);
	unlock_db();
   
}


void get_physical ( int bot)
{
	char query[0x100];
	sprintf(query,"select physical, enabled from bot where logical=%d", bot);
	DBread(query);

	if (dbres) {
		row=mysql_fetch_row(res_set);
		sscanf( row[0], "%d", &physical );
		sscanf( row[1], "%d", &enabled  );
	}
    unlock_db();
	mysql_free_result(res_set);
	physical_address[bot]=physical;
	bot_enabled[bot]=enabled;

}



void pattern_move(int bot, int step)
{
// 
	//char s[0x100];

	get_pattern_id(bot); 
	get_pattern_details(patternID,step);  // sets logical_target for this patternID and step
	// now we should have logical address, position,speed for this step
	
	physical=physical_address[logical_target];
	enabled =bot_enabled[logical_target];

	//sprintf(s,"physical=%d enabled=%d position=%d speed=%d",physical,enabled,position,speed);
	//logs(s);       
	usleep (speed*2000);         
	sprintf(s, "[bot] %d speed delay %d", logical_target, speed);   
	logs(s);
	
	if (enabled) {
		OribotCommand(3,physical,position,speed);
	}

}



const unsigned int group_a[] = {  1, 2, 3, 4,  5 };
const unsigned int group_b[] = {  6, 7, 8, 9, 10 };
const unsigned int group_c[] = {  11, 12, 13, 14, 15 };
const unsigned int group_d[] = {  16, 17, 18, 19, 20 };
const unsigned int group_e[] = {  21, 22, 23, 24, 25 };

const unsigned int group_f[] = {  26, 27, 28, 29, 30 };
const unsigned int group_g[] = {  31, 32, 33, 34, 35 };
const unsigned int group_h[] = {  36, 37, 38, 39, 40 };
const unsigned int group_i[] = {  41, 42, 43, 44, 45 };
const unsigned int group_j[] = {  46, 47, 48, 49, 50 };

void group_1 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_a[i],0,50);
		usleep(200000);
	}
}
void group_2 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_b[i],0,50);
		usleep(200000);
	}
}
void group_3 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_c[i],0,50);
		usleep(200000);
	}
}
void group_4 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_d[i],0,50);
		usleep(200000);
	}
}
void group_5 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_e[i],0,50);
		usleep(200000);
	}
}
void group_6 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_f[i],0,50);
		usleep(200000);
	}
}
void group_7 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_g[i],0,50);
		usleep(200000);
	}
}
void group_8 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_h[i],0,50);
		usleep(200000);
	}
}
void group_9 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_i[i],0,50);
		usleep(200000);
	}
}
void group_10 ( void ) {
	int i;
	for (i=0;i<5;i++) {
		OribotCommand(1,group_j[i],0,50);
		usleep(200000);
	}
}

void ScanNetwork ( int x, int y ) 
{
	int i;
	for (i=x;i<y+1;i++) {
		OribotCommand(0,i,0,0);
	}
}

void Cycle ( int x, int y) 
{
	int i;
	while (1) {
		for (i=x;i<y+1;i++) {
			OribotCommand(1,i,0,0);
		}
	}
}

void CycleT ( int x, int y) 
{
	int i;
	while (1) {
		if (x==y) {
				OribotCommand(3,x,0,20);
				usleep(1000000);
		}
		else {
			for (i=x;i<y+1;i++) {
				OribotCommand(3,i,0,20);				
			}
			usleep(1000000);
		}
	}
}

void red_wall ( void ) 
{
	int i;
	for (i=0;i<4;i++) {
		OribotCommand(3,63,0,60); 
		usleep(80000);
	}

}

void scan_for_triggers_1 (void)
{
	int i,j;
	char s[0x100];

	for (i=1;i<number_of_bots;i++) {
				
		OribotCommand(0,i,0,0);  // update position

		if (RxPos[i]<30) {
			if (i==35) { red_wall(); }
			sprintf(s,"*** Pattern Triggered by %d",i);
			logs(s); 
			get_pattern_num_steps (i);     
			sprintf(s,"*** Pattern Number of steps %d",pattern_num_steps);
			for (j=1;j<pattern_num_steps;j++) {  
				sprintf(s,"*** Pattern Move Triggered step %d",j); 
				pattern_move(i,j);
			}
		}
	}
}


void print_scan_order (void) 
{
	int i;
	printf("\n");
	printf("01..10> ");  for (i= 1;i<11;i++) { printf("%d ",scan_order[i]); } printf("\n");
	printf("11..20> ");  for (i=11;i<21;i++) { printf("%d ",scan_order[i]); } printf("\n");
	printf("21..30> ");  for (i=21;i<31;i++) { printf("%d ",scan_order[i]); } printf("\n");
	printf("31..40> ");  for (i=31;i<41;i++) { printf("%d ",scan_order[i]); } printf("\n");
	printf("41..50> ");  for (i=41;i<51;i++) { printf("%d ",scan_order[i]); } printf("\n");
	printf("\n");
}


void generate_scan_order (void )
{
	int k;
	char query[0x100];

	sprintf(query,"select physical from bot order by rand()");
	DBread(query);
	k=1;
	if (dbres) {
  		while((row = mysql_fetch_row(res_set))) {
				sscanf(row[0],"%d",&scan_order[k]);
				//printf("%d %d \n", k,scan_order[k]);
				k++;
  		}
	}
    unlock_db();
	mysql_free_result(res_set);
	//print_scan_order();
}

void save_position_to_db ( int bot, int pos) 
{
	char query[0x100];
	sprintf(query,"update bot set position = '%d' where physical='%d'", pos,bot);
	DBupdate( query);

}
  

void execute_pattern (int bID)
{
	int j;
	sprintf(s,">>> [Execute_pattern] for %d",bID);
	logs(s);
	
	// get the number of steps required
	get_pattern_num_steps (bID);     
	sprintf(s,">>> Pattern Number of steps %d",pattern_num_steps);
	// iterate over steps for the bot pattern
	
	for (j=1;j<pattern_num_steps;j++) {  
		sprintf(s,">>> Pattern Move Triggered step %d",j); 
		pattern_move(bID,j);
	}
}


void poll_node ( int p) 
{
		char s[0x100];

		//printf("Polling Node %2d Position %3d ..", p, RxPos[p]);
		OribotCommand(0,p,0,0);  // update position
	    save_position_to_db(p,RxPos[p]);
		if (RxPos[p]<20) {
			if (p==35) { red_wall(); }
			sprintf(s,"*** Pattern Triggered by %d in {poll_node}",p);
			logs(s);                                  
			execute_pattern(p); // trigger the pattern
		}
}

unsigned int priority_list[100];

void priority_scan ( void ) 
{
	int p,i,k;
	char query[0x100];	
	char s [0x100];

	for (i=0;i<100;i++) { priority_list[i]=0; }
	i=0;
	sprintf(query,"select physical from bot where position < 60 order by position +0 limit 5");
	DBread(query);
	if (dbres) {
  		while((row = mysql_fetch_row(res_set))) {
				sscanf(row[0],"%d",&p);
				sprintf(s,"Priority %d Distance %d \n",p, RxPos[p]);
				logs(s);
				priority_list[i]=p;
				i++;
  		}
	}
    unlock_db();
	mysql_free_result(res_set);
	for (k=0;k<i;k++) {
		poll_node( priority_list[k]);
	}	
}

void scan_for_triggers (void)
{
	int i;


	for (i=1;i<number_of_bots;i++) { 
	
		poll_node(scan_order[i]);
		
		if ((i%8)==0) { priority_scan(); }

	}
}


        

int check_for_actions (int ctr)
{        
   
    int bID;  
	int id;
	char action;
	char query[0x100];	
	char s [0x100]; 

	sprintf(query,"select id,botID,action from actions");
	DBread(query); 
	unlock_db();
	sprintf(s,">>>>>>>>>>>>>>>>>>>>>>>>>>>>> actions db");
	logs(s);
	
	if (dbres) {          
			
			while((row = mysql_fetch_row(res_set)))
			{
			
	   	 	sscanf(row[0],"%d",&id);
			sscanf(row[1],"%d",&bID);
			sscanf(row[2],"%s",&action);    
			
			sprintf(s,">>>>>>>>>>>>>>>>>>>>>>>>>>>>> actions all data");
			logs(s);
				switch (action)
				{
					case 'P':      // ping
						sprintf(s,">>>>>>>>>>>>>>>>>>>>>>>>>>>>> Pinging bot %d",bID);
						logs(s); 
						OribotCommand(3,bID,10,100);
						break;

					case 'S':    // scan
						sprintf(s,">>>>>>>>>>>>>>>>>>>>>>>>>>>>> Checking status of all bots");
						logs(s); 
						reset_bot_enables();
						ctr = 2; // ctr value returned is one step before check_enables is run;
						break;  
					
						/*  CAUSE STACK OVERFLOW
						 
						 
					case 'D':    // scan
						sprintf(s,">>>>>>>>>>>>>>>>>>>>>>>>>>>>> Draw bot for bot %d", bID);
						
						int j;
						sprintf(s,">>> [Execute_pattern] for %d",bID);
						logs(s);
						
						// get the number of steps required
						get_pattern_num_steps (bID);     
						sprintf(s,">>> Pattern Number of steps %d",pattern_num_steps);
						// iterate over steps for the bot pattern
						
						for (j=1;j<pattern_num_steps;j++) {  
							sprintf(s,">>> Pattern Move Triggered step %d",j); 
							pattern_move(bID,j);
						}
						
						
						break; */
						
						
					default:
						break;
						
					
				}     
			}
	}   
	
	// delete the data
	mysql_free_result(res_set); 
	unlock_db();
	 
	// empty the table    
	sprintf(query,"truncate table actions");   
	mysql_query(conn,query);   
	return ctr;
}


void help ()
{
	printf ("\n Command line optons for AEC Oribotics 2010");
	printf ("\n ");
	printf ("\n  -V             ... Print Version number and exit  ");
	printf ("\n  -v             ... verbose echo log to local terminal");
	printf ("\n Bot Commands");
	printf ("\n  -R xx          ... Read Position from node xx ");
	printf ("\n  -W xx dd ss    ... Position node xx to dd time ss");
	printf ("\n  -G aa          ... Poll a group of 5 aa=1,2,3,4,5,6,7,8,9 or 10 ");
	printf ("\n  -P xx          ... Test Pattern for bot xx");
	printf ("\n  -S             ... Scan Network from 1 to 50");
	printf ("\n  -C xx yy       ... Test Network from xx to yy");
	printf ("\n  -T xx yy       ... Test Network from xx to yy");
	printf ("\n  -?             ... Help... but you already knew that!");
    printf ("\n");
}

int main(int argc, char *argv[])
{
	int i;
	char s[0x100];
	int ctr;
	int node_address;
	int node_position;
	int node_timer;
	int group;
	int x,y;

	help();
	
	strcpy(logfilename,"/var/log/oribotics.log");
	
	// default state : logging and verbose = 0	
	verbose=0;
	logging=0;

	sprintf(s,"%s",version);
	logs(s);
	

	terminal_mode=0;
	add_linefeed=1;
	hexdump=1;
	echo=1;

	ctr=0;

	while ( ConnectToDataBase()==0) { 
		logs("Retrying connection in 5 seconds..");
		usleep(5000000);
	}

	for (i=1; i<argc; i++)
	{
	 	if (argv[i]==NULL) { continue; }
		if ((argv[i][0]=='-') && (argv[i][2]=='\0'))
		{
            		// single letter preceeded by '-'
			switch (argv[i][1])
			{
			case 'V': 		printf("\n %s \n\n",version ); 
						exit(0); 
						break;

			case 'v': 	verbose=1;
						break; 
			case 'l': 	logging=1;                  	
						break;

                	case 'R': 		sscanf( argv[i+1], "%d", &node_address);  
						InitComms();
						OribotCommand(0,node_address,node_position,0);
						goto byebye;
						break;

			case 'W': 		sscanf( argv[i+1], "%d", &node_address);
						sscanf( argv[i+2], "%d", &node_position);  
						sscanf( argv[i+3], "%d", &node_timer);
						InitComms(); 
						if (node_address==63) {		
							for (i=0;i<4;i++) {
								OribotCommand(3,node_address,node_position,node_timer); 
								usleep(100000);
							}
						}
						else {
							OribotCommand(3,node_address,node_position,node_timer); 
						}
						goto byebye;
						break;

			case 'G': 		sscanf( argv[i+1], "%d", &group);
						InitComms();
						switch (group) {
						   case 1: group_1();  break;
						   case 2: group_2();  break;
						   case 3: group_3();  break;
						   case 4: group_4();  break;
						   case 5: group_5();  break;
						   case 6: group_6();  break;
						   case 7: group_7();  break;
						   case 8: group_8();  break;
						   case 9: group_9();  break;
						   case 10:group_10(); break;
						   default: break;						  
						}						 
						goto byebye;
						break;

			case 'P':	sscanf( argv[i+1], "%d", &x);
						exit(0);
						break;

			case 'S':		sscanf( argv[i+1], "%d", &x);
						sscanf( argv[i+2], "%d", &y);
						InitComms();
						ScanNetwork(x,y);
						goto byebye;
						break;

			case 'C':		sscanf( argv[i+1], "%d", &x);
						sscanf( argv[i+2], "%d", &y);
						InitComms();
						Cycle(x,y);
						goto byebye;
						break;
			case 'T':		sscanf( argv[i+1], "%d", &x);
						sscanf( argv[i+2], "%d", &y);
						InitComms();
						CycleT(x,y);
						goto byebye;
						break;
			case 'Z':		 
						InitComms();
						while (1) {
							red_wall();
							usleep(500000);
						}
						goto byebye;
						break;

			case '?':
						help();
						exit(0);
						break;
				
		    	default:
						printf ("\n Error: %s is not a valid option!\n\n", argv[i] );
						help();
						exit(0);
						break;
			}
		}
	}


	if (terminal_mode) {
		InitConsole();
		// useful for interactively typing at commands to devices
		// while debugging 
		sprintf(s,"Switching to Terminal Mode");
		logs(s);
		
		serial_terminal(add_linefeed, hexdump,echo);
		
		sprintf(s,"Leaving Terminal Mode");
		logs(s);
		RestoreConsoleTermios();
	}
	else {
		sprintf(s,"Not in Terminal Mode");
		logs(s);
		InitComms();
		ConnectToDataBase(); 
		update_bot_enables();
		generate_scan_order();
		for (i=1;i<number_of_bots;i++) { poll_node(i); }

		for (;;) {
			if (ctr==3) { update_bot_enables(); } 
			ctr = check_for_actions(ctr); 
			generate_scan_order();
			scan_for_triggers();
			ctr++;
		}
		goto byebye;
	}

byebye:
	RestoreSerialTermios();
   	printf("\n");  
   	return EXIT_SUCCESS;
}
