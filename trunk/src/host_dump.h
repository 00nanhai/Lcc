#ifndef __HOST_RECORDS_H__
#define __HOST_RECORDS_H__

#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <event.h>
#include <pthread.h>
#include <evhttp.h>
#include <event2/thread.h>
#include <errno.h>
#include <string>
#include <map>

#include "global.h"
#include "mydb_instance.h"

using namespace std;

class host_records
{
public :
	static pthread_t loop_tid ; 
	static pthread_mutex_t host_mutex ;
public :	
	host_records()
	{
	}	

	bool start( );
public :
	static void* loop(void*);
	static void dump(int );
	static void tar_dump_files(vector<string> &host_files);
	static bool dump_host_record(map<string, FILE*> &host_files,const string &host,const string &ckey,const string &curl,const string &size,const string &error,const string &tb_name);

private:
	static bool dump_tbdata_to_files(const string &tb_name);
	//return files path in @path (the dir in @path will be ommited) 
	static bool list_files(const string &path, vector<string> &files_path);
};

#endif  /*__HOST_RECORDS_H__*/

