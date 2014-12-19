#ifndef mydb_operations_H
#define mydb_operations_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <err.h>
#include <event.h>
#include <evhttp.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <event2/thread.h>

#include <errno.h>
#include <signal.h>
#include "boost/lexical_cast.hpp"
#include "../global.h"

using namespace std;

#define get_base_conf_item(key) g_base_conf->get(key)

class mydb_operations
{
public :
	/*Wurl<->Ckey*/
	static bool get_ckey_from_wurl(const string &wurl, const std::string &tb_name, string &ckey, MYSQL *con);
	static bool get_ckey_from_id(const string &id, const std::string &tb_name, string &ckey, MYSQL *con);
	static bool set_wurl_ckey(const string &wurl,const string &ckey, const std::string &tb_name, MYSQL *con);

	/*Delete Curl*/
	static bool del_curl(const string &curl, const std::string &tb_name, vector <string> &wurls, string &ret, MYSQL *con);
    static bool delay_del(const string &host, const string &tb_name, vector <string> &wurls, string &ret, MYSQL *con);
    static bool delay_del_report(const string &curl, const string &host, const string &tb_name, string &ret, MYSQL *con);

    static bool get_resource_size_by_curl(const string &curl, const string &tb_name, string &ret, MYSQL *con);

	/*Ckey<->Curl*/
	static bool get_record_from_ckey(const string &ckey ,const std::string &tb_name, string &curl, MYSQL *con);	
	static bool set_ckey_curl(
		const string &ckey, const string &curl,
		const string &host, const string &size,
		const string &wurl, const std::string &tb_name, string &ret, MYSQL *con);
	static bool set_ckey_curl_have_id(/*已废弃*/
		const string &ckey, const string &curl,
		const string &host, const string &size,
		const string &wurl, const string &id,
		const std::string &tb_name, string &ret, MYSQL *con);
	/**/
	static bool exec_sql(const char *sql,string &ret, MYSQL *conn);
};

#endif  /*mydb_operations_H*/

