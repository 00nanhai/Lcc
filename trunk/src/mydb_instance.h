#ifndef __MYDB_INSTANCE_H__
#define __MYDB_INSTANCE_H__
#include <stdio.h>
#include <iostream>
#include <mysql/mysql.h>
#include "global.h"

#include "slib/config.h"
#include "slib/path.h"
#include "slib/smutex.h"
#include "slib/slock.h"
#include "slib/config.h"

#include <my_log.h>
using namespace std;
using namespace slib ;

bool get_instance(MYSQL *my_db);
void destroy_connect(MYSQL *my_db);
void init_mysql_library();
void release_mysql_library();
void init_mysql_thread();
void release_mysql_thread();
#endif


