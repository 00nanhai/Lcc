#include <signal.h>
#include <assert.h>
#include "global.h"
#include "signals.h"

void signal_hooker::set_signals( )
{
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	return ;
}

void signal_hooker::signal_handler(int sig) 
{
    switch (sig) 
    {
         case SIGTERM:
         case SIGHUP:
         case SIGQUIT:
         case SIGINT:
				exiting = 1;
    }
	return ;
}

