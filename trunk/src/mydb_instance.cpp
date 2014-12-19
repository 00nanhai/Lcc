#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <mysql/mysql.h>

#include "common.h"
#include "global.h"

#include "slib/config.h"
#include "slib/path.h"
#include "slib/smutex.h"
#include "slib/slock.h"
#include "slib/config.h"
#include "mydb_instance.h"

#include <my_log.h>
using namespace std;
using namespace slib;

bool get_instance(MYSQL *my_db)
{
	Assert(my_db);

	std::string host = g_base_conf->get("db_host");
	int port = atoi(g_base_conf->get("db_port").c_str());
	std::string user = g_base_conf->get("db_user");
	std::string passwd = g_base_conf->get("db_passwd"); 
	std::string db = g_base_conf->get("db_name");
	std::string chatset = g_base_conf->get("db_charset");

	LOG_INFO("the safe:%d", mysql_thread_safe());
	assert(mysql_init(my_db) != NULL);
	//assert(mysql_thread_init() == 0);
	// set automatic reconnect 
	char _rc = 1;
	mysql_options(my_db, MYSQL_OPT_RECONNECT, &_rc);

	// set charset 
	mysql_options(my_db, MYSQL_SET_CHARSET_NAME, chatset.c_str() );

	// real connect 
	if( !mysql_real_connect(
				my_db, 
				host.c_str(), 
				user.c_str(), 
				passwd.c_str(), 
				db.c_str(),
				port, 
				NULL, 
				0))
	{
		std::string error = "conect " + host + " " + db + mysql_error(my_db);
		LOG_ERRO("%s", error.c_str());
		//TODO
		cout << error << endl;
		return false;
	}

	const char *sql = "set names 'latin1';";
	if (mysql_query(my_db, sql)) {
		LOG_ERRO("mysql_query [%s] failed: %d: %s", 
				sql, mysql_errno(my_db), mysql_error(my_db));
		return false;
	}

	return true;
}

void destroy_connect(MYSQL *my_db)
{
	Assert(my_db);
	mysql_close(my_db);
}

void init_mysql_library()
{
	mysql_library_init(0, NULL, NULL);
}

void release_mysql_library()
{
	mysql_library_end();
}

/*TODO for mysql secure thread api*/
void init_mysql_thread()
{
	 mysql_thread_init();
}

void release_mysql_thread()
{
	mysql_thread_end();
}


