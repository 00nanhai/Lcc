#ifndef _LCC_SIGNALS_H_
#define _LCC_SIGNALS_H_

#include <stdio.h> 
#include <signal.h>   
#include <sys/time.h>  
#include <errno.h> 

class signal_hooker
{
public :
	static void set_signals( );
	static void signal_handler(int sig) ;
} ;

#endif
