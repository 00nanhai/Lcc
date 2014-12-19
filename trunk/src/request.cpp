#include <iostream>
#include <string>
#include <map>
#include <stdlib.h>
#include <dirent.h>
#include "boost/lexical_cast.hpp"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp> 

#include <event.h>
#include <evhttp.h>
#include <my_log.h>

#include "mysql_worker.h"
#include "config.h"
#include "global.h"
#include "host_dump.h"
#include "slib/path.h"
#include "slib/utils.h"
#include "slib/json.h"
#include "slib/xxtea.h"
#include "request.h"
#include "evhttp_opt.h"
#include "response.h"


using namespace std;
using namespace slib;
using namespace boost;

//pthread_mutex_t  lcc_request::request_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t  lcc_request::update_reflect_list_mutex = PTHREAD_MUTEX_INITIALIZER;
/*
* Parse out the params in uri into the map<string , string>
*/
void lcc_request::parse_params(char * uri, map<string,string> &params)
{
	char *p = uri;
	p = strchr(p,'?');
	if (p == NULL)
		return;

	char *k = p+1;
	char *v, *e;
	while(k!= NULL && *k)
        {
		e = strchr(k,'&');
		if(e != NULL)
		{
			*e = '\0';
			e ++;
		}

		v = strchr(k,'=');
		if(v != NULL)
		{
			*v = '\0';
			v++;
			params[string(k)] = string(v);
		}

		k = e;
	};
}

/*
 *  Get necessary and hash fields in params prepare for the check_lcc_request( )
 */
void lcc_request::get_necessary_and_hash_params_by_op(
	const int i_op , vector<string> &necessary_params, 
	vector<string> &hash_params )
{
	//EEIWANT NEED CHANGE necessary_params.push_back("tbname")
	switch( i_op )
	{
		case GET_CKEY :	
			necessary_params.push_back("wurl");
			necessary_params.push_back("tb_name");
			break;
		case SET_CKEY :
			necessary_params.push_back("wurl");
			necessary_params.push_back("ckey");
			necessary_params.push_back("hash");
			necessary_params.push_back("tb_name");

			hash_params.push_back("wurl");
			hash_params.push_back("ckey");
			break;
		case GET_CURL :
			necessary_params.push_back("ckey");
			necessary_params.push_back("tb_name");
			break;
		case SET_CURL :
			necessary_params.push_back("wurl");
			necessary_params.push_back("ckey");
			necessary_params.push_back("curl");
			necessary_params.push_back("host");
			necessary_params.push_back("size");
			necessary_params.push_back("hash");
			
			// wurl_str+ckey+curl+host+size, crypt_str			
			hash_params.push_back("wurl");
			hash_params.push_back("ckey");
			hash_params.push_back("curl");
			hash_params.push_back("host");
			hash_params.push_back("size");
			hash_params.push_back("tb_name");
			break;
		case DEL_CURL :
			necessary_params.push_back("curl");
			necessary_params.push_back("hash");

			hash_params.push_back("curl");
			hash_params.push_back("tb_name");
			break;
			break;
		case DUMP_HOST_RECORDS :
			necessary_params.push_back("host");
			break;
		case CREATE_B_FILE:
			break; 
		case GET_FILE:
			necessary_params.push_back("name"); 
			break;
		case GET_CKEY_BY_ID:/*已废弃*/
			necessary_params.push_back("id");
			necessary_params.push_back("tb_name");
			break;
		case SET_CURL_HAVE_ID:/*已废弃*/
			necessary_params.push_back("wurl");
			necessary_params.push_back("id");
			necessary_params.push_back("ckey");
			necessary_params.push_back("curl");
			necessary_params.push_back("host");
			necessary_params.push_back("size");
			necessary_params.push_back("hash");
			necessary_params.push_back("tb_name");
			break;
		case CHECK_FILE:
			break;
        case GET_RESOURCE_SIZE:
			hash_params.push_back("host");
			necessary_params.push_back("curl");
			hash_params.push_back("tb_name");
            break;
		case DELAY_DEL :
            //necessary_params.push_back("curl");
			//necessary_params.push_back("hash");
			necessary_params.push_back("host");
			necessary_params.push_back("tb_name");

			hash_params.push_back("host");
			//hash_params.push_back("curl");
			hash_params.push_back("tb_name");
			break;

        case DELAY_DEL_REPORT:
            //necessary_params.push_back("hash");
            necessary_params.push_back("host");
            necessary_params.push_back("tb_name");
            necessary_params.push_back("curl");

			hash_params.push_back("curl");
			hash_params.push_back("host");
			hash_params.push_back("tb_name");
            break;

		default :
			LOG_ERRO("get_necessary_and_hash_params_by_op fail. %d operate no support.", i_op);
			break;
	}
	return;
}

int g_request = 0;
bool lcc_request::check_lcc_request(const int i_op , 
	const map<string,string> & request, 
	string & check_result )
{
	++g_request;
	// 1) collect necessary params 
	// 2) collect hash params  (sort )
	vector<string> necessary_params , hash_params;
	get_necessary_and_hash_params_by_op(i_op, necessary_params , hash_params );
	vector<string>::iterator item;

	// check necessry params 
	for( item = necessary_params.begin(); item !=necessary_params.end();
		item ++ )
	{
		if (request.find(*item) == request.end())
		{
			char temp[2014];
			snprintf(temp,sizeof(temp),
				"No [%s] param in request when operation=%d",(*item).c_str(), i_op);
			check_result = string(temp);
			return false;
		}
	}

#ifdef USE_HASH_REQUEST		
	// check hash 
	return true;
	if( hash_params.size() > 0 )
	{
		string total_hash_str;
		for( item = hash_params; item != NULL;
			item ++ )
		{
			total_hash_str.append( request[*item] );
		}

		string hash_str;
		string crypt_str;
		
		g_cryptor.encrypt(total_hash_str, crypt_str);
		SHA1(crypt_str, hash_str);

		if( hash_str != request["hash"] )
		{
			check_result = "hash field is wrong.";
			return false;
		}
	}
#endif
	return true;
}

static int operate_code( map<string,string> &params)
{
	string op = params["operation"];
	if (op.empty())
		return  -1;

	const char *p;
	string result;
	for (p = op.data(); *p != 0; p++) {
		if (!isdigit(*p)) {
			result = "Invalid operation value :" + op;
			LOG_ERRO("%s. ", result.c_str());
			return -1;
		}
	}	

	int i_op = atoi(op.c_str());
	if (i_op >= LAST_RESERVE_OPT) {
		result = "Invalid operation value :" + op;
		LOG_ERRO("%s. ",result.c_str());
		return  -1;
	}
	return i_op;
}

void lcc_request::location_cc_handler(struct evhttp_request *req, void *arg)
{
	/* get http request */
	struct evbuffer *buf;
	char *decode_uri;
	char *ip;
	uint16_t port;
	evhttp_get_data(req, &ip, &port,  &decode_uri, &buf);
	LOG_INFO("[%s:%d] requet : %s.", ip, port, decode_uri);

	map<string,string> params;
	parse_params(decode_uri, params);

	int i_op;
	string result;
	if ((i_op = operate_code(params)) == -1) {
		evhttp_replay_bad_request(req, buf, 
									"operate code error.", 
									decode_uri);
		goto END;
	}
	
	if (!check_lcc_request(i_op, params, result)) {
		evhttp_replay_bad_request(req, buf, result.c_str(), decode_uri);
		goto END;
	}
	response(req, buf, params, i_op);
END:
	free(decode_uri);
	evhttp_free_buf(buf);
	return;
}


