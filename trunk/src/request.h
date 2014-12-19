#ifndef LCC_REQUEST_H
#define LCC_REQUEST_H

#include<iostream>
#include<string>
#include<stdlib.h>
#include<map>
#include "boost/lexical_cast.hpp"
#include "slib/config.h"
#include "mysql_worker.h"
#include "signals.h"
#include <err.h>
#include <event.h>
#include <pthread.h>
#include <evhttp.h>
#include <event2/thread.h>
#include <errno.h>
#include <signal.h>
#include "global.h"
//#include "reflect_list.h"

using namespace std ;
using namespace boost ;

/* name :lcc_request
 *
 * This class check and parse the http request , The request typs includes :
 	GET_CKEY = 0 ,
	SET_CKEY ,
	GET_CURL ,
	SET_CURL ,
	DEL_CURL ,
	LCC_REQ_COUNT ,	
 *
 * After parse the request , dispatch the operation and params to backand database .
 * 
 * guoyuejing@datafrog.com 2013-11-16
 */
class lcc_request 
{
public  :
	//static pthread_mutex_t request_mutex ;
	//static pthread_mutex_t update_reflect_list_mutex ;
private :
	static void parse_params(char * uri, map<string,string> &params);
	static void get_necessary_and_hash_params_by_op(
				const int i_op , vector<string> &necessary_params, 
				vector<string> &hash_params );
	static bool check_lcc_request(const int i_op ,  const map<string,string> & request , string & check_result );
	//static void send_file(struct evbuffer *buf, struct evhttp_request *req, const string &path);
	//static void response( const int i_op , string & result , map<string,string> & params );
	
public :
	static void location_cc_handler(struct evhttp_request *req, void *arg); 
};

#endif

