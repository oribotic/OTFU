/***************************************************************************
                          main.h  -  description
                             -------------------
    begin                : Wed Jun 19 2002
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/termios.h>
#include <linux/serial.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <math.h>
#include <float.h>
#include <signal.h>
#include <pthread.h>
#include <sys/syslog.h>

extern int logging;
extern int verbose;
extern char logfilename[];
extern void update_bot_timedout (unsigned char, unsigned char);

#include "serial_support.h"
#include "logging.h"

#endif
