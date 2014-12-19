#include <stdio.h>
#include <iostream>
#include <mysql/mysql.h>

#include "common.h"
#include "global.h"
#include "file_opt.h"
#include "mydb_instance.h"
#include "mysql_worker.h"
#include "create_B.h"

static MYSQL *g_Afile_sql_connect;
static FILE *g_CAfile_fp = NULL;
static int g_CAfile_index = 0;
static long long g_CAfile_timestamp = 0;
static char g_CAfile_prefix[16] = {0};

bool static is_valid_append_file(const char *name)
{
	Assert(name);
	//eg : A20140101_0001
	if (APPEND_NAME_FLAG != *name)
		return false;
	if (name[9] != '_') {
		return false;
	}
	if (APPEND_NAME_LEN != strlen(name))
		return false;
	for (int i = 1; i < BASIC_NAME_LEN; i++) {
		if (i == 9) 
			continue;
		if (!isdigit(*(name + i)))
			return false;
	}
	/*FIXME 没有做更加严格的检查 */
	return true ;
}

int init_Afile()
{
	/* 初始化数据库实例, FIXME 没有free内存 */
	g_Afile_sql_connect = new MYSQL();
	/* 友好的错误处理 */
	Assert(g_Afile_sql_connect);
	Assert(get_instance(g_Afile_sql_connect));

	/* 查询最新创建的A文件 */
	char sql[MAX_SQL_SIZE] = {0};
	snprintf(sql, MAX_SQL_SIZE, 
			"SELECT * FROM %s.%s \
			WHERE timestamp = (SELECT MAX(timestamp) FROM %s.%s);",
			DB_NAME, AFILE_TABLE, DB_NAME, AFILE_TABLE);
	string exec_err_info;
	bool ret = mydb_operations::exec_sql(sql, 
								exec_err_info, 
								g_Afile_sql_connect);
	if (!ret) {
		LOG_ERRO("init_Afile fail. exec_sql:%s", sql);
		exit(0);
	}

	/* 读取结果集 */
	MYSQL_RES *res = mysql_use_result(g_Afile_sql_connect);
	assert(res);
	MYSQL_ROW row = mysql_fetch_row(res);
	if (row == NULL) {
		return 0;
	}

	long long timestamp;
	int index, status;
	char name[512], prefix[16];
	strncpy(name, row[0], 512);
	strncpy(prefix, row[1], 16);
	timestamp = atoll(row[2]);
	index = atoi(row[3]);
	status = atoi(row[4]);

	/* FIXME prefix 是否为最大, 相同prefix index是否最大 */
	/* name 和 prefix 是否合法 */
	/* 释放结果集 */
	mysql_free_result(res);

	/* 文件是否处于完成状态或创建时间超过5分钟,会在再次创建A文件时处理 */
	char path[1024] = {0};
	/* TODO 没有考虑当路径改变的情况,或者程序每次重启重新创建B文件和A文件 */
	joint_path_by_dir_and_name(path, REFLECT_LIST_AFILE_DIR, name);
	if (status == 0) {
		g_CAfile_fp = fopen(path, "a");
#ifdef _SHOW_INFO_
		if (g_CAfile_fp == NULL) {
			Perror(path);	
		}
#endif
	}
	/* 设置全局创建时间,index,prefix */
	g_CAfile_index = index;
	g_CAfile_timestamp = timestamp;
	strncpy(g_CAfile_prefix, prefix, APPEND_PREFIX_LEN);

	return 1;
}

static bool insert_Afile_info_to_BD(const char *Afile)
{
	char sql[MAX_SQL_SIZE] = {0};
	snprintf(sql, MAX_SQL_SIZE, 
			"insert into %s.%s values('%s','%s',%lld, %d, 0);",
			DB_NAME, AFILE_TABLE, Afile, g_CAfile_prefix, 
			g_CAfile_timestamp, g_CAfile_index);
	string exec_err_info;
	bool ret = mydb_operations::exec_sql(sql, 
								exec_err_info, 
								g_Afile_sql_connect);
	//cout << sql << endl;
	if (!ret) {
		LOG_ERRO("insert_Afile_info_to_BD:%s", exec_err_info.c_str());
		return false;
	}
	return true;
}

static bool update_Afile_status(long long timestamp)
{
	char sql[MAX_SQL_SIZE] = {0};
	snprintf(sql, MAX_SQL_SIZE, 
			"update %s.%s set status = 1 where timestamp = %lld;",
			DB_NAME, AFILE_TABLE, timestamp);
	string exec_err_info;
	bool ret = mydb_operations::exec_sql(sql, 
								exec_err_info, 
								g_Afile_sql_connect);
	if (!ret) {
		LOG_ERRO("update_Afile_status:%s", exec_err_info.c_str());
		return false;
	}
	return true;
}

static bool is_Bfile_update(const char *Afile_prefix)
{
#if 0
	string Bfile;
	/*FIXME　B文件一定应该存在, 若B文件真的不存在,可以重新创建B文件*/
	//B20140101->A20140101
	Assert(get_cur_month_newest_Bfile(Bfile, REFLECT_LIST_BASIC_DIR));
	int Adate = atoi(Afile_prefix + 1);
	int Bdate = atoi(Bfile.c_str() + 1);
	/* 有可能存在B的号比A小 */
	if (Bdate != Adate) {
		return true;
	}
	//Assert(Bdate == Adate);
	return false;
#else
    string Bname;
    int Bindex;
    read_globle_Bfile(Bname, Bindex);

    if (strncmp(Afile_prefix + 1, Bname.c_str() + 1, Bname.length() -1) != 0) {
        return true;
    } else {
        return false;
    }
#endif
}

static void create_Bfile()
{
    /* 不让A文件生成太多,文件多的时候校验A文件的完整性时会变慢 */
    if (g_CAfile_index >= 5000) {
		send_create_Bfile_signal();
		LOG_WARN("%s:%d Afile_index > 5000.", g_CAfile_prefix, g_CAfile_index);
        return;
    }

    /* 进入新的一月触发B文件 */
	struct tm tm_s;
	time_t now = time(NULL);
	char strdate[BASIC_NAME_LEN + 1] = {0};
	strftime(strdate, BASIC_PREFIX_LEN, "%Y%m", localtime_r(&now, &tm_s));
	int date = atoi(strdate);

    string Bname;
    int Bindex;
    read_globle_Bfile(Bname, Bindex);

    char buf[16] = {0};
    //B20140701->201407
    strncpy(buf, Bname.c_str() + 1, 6);
    int Bdate = atoi(buf);
    /*由于人为原因可能存在Bdate > date情况*/
	if (date != Bdate) {
		send_create_Bfile_signal();
		LOG_WARN("%s:%s Info new month, send create Bfile singnal.",
					strdate, Bname.c_str());
	}
}

static void get_newest_Afile_prefix_from_Bfile(char *Afile_prefix)
{
#if 0
	string Bfile;
	/*FIXME　B文件一定应该存在, 若B文件真的不存在,可以重新创建B文件*/
	Assert(get_cur_month_newest_Bfile(Bfile, REFLECT_LIST_BASIC_DIR));
	//B20140101->A20140101
	strncpy(Afile_prefix, Bfile.c_str(), APPEND_PREFIX_LEN);
	Afile_prefix[0] = 'A';
#else
    string Bname;
    int Bindex;
    read_globle_Bfile(Bname, Bindex);
	//B20140101->A20140101
	strncpy(Afile_prefix, Bname.c_str(), APPEND_PREFIX_LEN);
	Afile_prefix[0] = 'A';
#endif
}

//delete previous prefix Afile.
void rm_old_Afile()
{
	//get max prefix
	char sql[MAX_SQL_SIZE] = {0};
	snprintf(sql, MAX_SQL_SIZE, "SELECT MAX(prefix) FROM %s.%s;", 
				DB_NAME, AFILE_TABLE);
	
	string exec_err_info;
	bool ret = mydb_operations::exec_sql(sql, 
								exec_err_info, 
								g_Afile_sql_connect);
	Assert(ret);
	MYSQL_RES *res = mysql_use_result(g_Afile_sql_connect);
	Assert(res);
	MYSQL_ROW row = mysql_fetch_row(res);
	if (row == NULL) {
		LOG_ERRO("Not Found Afile.");
		return;
	}

	if (row[0] == NULL) {
		LOG_ERRO("row[0] Not Found Afile.");
		mysql_free_result(res);
		return;
	}
	//achar max_Afile_prefix[APPEND_PREFIX_LEN + 1] = {0};
	//strncpy(max_Afile_prefix, row[0], APPEND_PREFIX_LEN);
	if (strcmp(row[0], g_CAfile_prefix) != 0) {
		/* 认为删除A文件,却没有清空A文件 */
		LOG_WARN("db max_afile_perfix:%s g_CAfile_prefix:%s", 
				row[0], g_CAfile_prefix);
	}
	mysql_free_result(res);

	//get old file name. 
	//snprintf(sql, MAX_SQL_SIZE, "SELECT name FROM %s.%s WHERE prefix < '%s';", 
	/* 防止误操作导致A文件的index大于B文件的index,或数据中存在比当前index大的值 */
	snprintf(sql, MAX_SQL_SIZE, 
				"SELECT name FROM %s.%s WHERE prefix != '%s' OR `index` > %d;", 
				DB_NAME, AFILE_TABLE, g_CAfile_prefix, g_CAfile_index);
	ret = mydb_operations::exec_sql(sql, 
									exec_err_info, 
									g_Afile_sql_connect);
	if (!ret) {
		LOG_ERRO("select fail. %s", sql);
		return;
	}

	res = mysql_store_result(g_Afile_sql_connect);
	if (res == NULL) {
		LOG_INFO("Not old Afile.");
		return;
	}

	if (mysql_num_rows(res) <= 0) {
		mysql_free_result(res);//no exist old file.
		return;
	}
	//delete file
	char name[256] = {0}; 
	while ((row = mysql_fetch_row(res)) != NULL) {
		snprintf(name, 256, "%s.tgz", row[0]);
		rm_file(REFLECT_LIST_AFILE_DIR, name);
	}
	mysql_free_result(res);
	//del db info
	snprintf(sql, MAX_SQL_SIZE, 
				"DELETE FROM %s.%s WHERE prefix != '%s' OR `index` > %d;", 
				DB_NAME, AFILE_TABLE, g_CAfile_prefix, g_CAfile_index);
	ret = mydb_operations::exec_sql(sql, 
									exec_err_info, 
									g_Afile_sql_connect);
	if (!ret) {
		LOG_ERRO("delete old afile fail:%s", sql);
	}
	LOG_INFO("Delete previous %s Afile.", g_CAfile_prefix);
}

static void make_and_open_Afile()
{
	char Afile_prefix[16] = {0};
	/* 查看B文件是否更新 */
	/* FIXME g_CAfile_index[0] = '\0'*/
	if (g_CAfile_prefix[0] == '\0' || is_Bfile_update(g_CAfile_prefix)) {
		get_newest_Afile_prefix_from_Bfile(Afile_prefix);
		strncpy(g_CAfile_prefix, Afile_prefix, APPEND_PREFIX_LEN);
		g_CAfile_index = 1;
	} else {
		g_CAfile_index++;
		Assert(g_CAfile_index <= 9999);
	}

	/* 创建A文件 */
	char Afile_name[32] = {0};
	snprintf(Afile_name, 32, "%s_%04d", g_CAfile_prefix, g_CAfile_index);
	Assert(is_valid_append_file(Afile_name));

	/* 创建A文件 */
	char path[512] = {0};
	Assert((strlen(REFLECT_LIST_AFILE_DIR) + strlen(Afile_name)) < sizeof(path));
	joint_path_by_dir_and_name(path, REFLECT_LIST_AFILE_DIR, Afile_name);
	/* FIXME 处理创建失败情况 */
	Assert(touch_afile(path) == 0);

	/* 入库 FIXME:入库错误处理 */
	g_CAfile_timestamp = time(NULL);
	if (!insert_Afile_info_to_BD(Afile_name)) {
		/* 可能数据库没有清理,导致A文件的信息已经在数据库中 */
		/* 若已完成的文件为未完成状态,会在检查A不连续,增加B文件index,解决该问题 */
		LOG_ERRO("create Afile insert DB status.");
	}

	g_CAfile_fp = fopen(path, "w");
	if (g_CAfile_fp == NULL) {
		Perror(path);
		Assert(0);//for debug
	}

	rm_old_Afile();
}

static long long pass_timestamp(long long old)
{
	long long cur = (long long)time(NULL);
    if (cur < old) {
	    LOG_WARN("cur < old %lld:%lld", cur, old);
        return 0;
    }
	return cur - old;
}

static void close_old_Afile()
{
	fclose(g_CAfile_fp);
	g_CAfile_fp = NULL;
	char Afile_name[32] = {0};
	snprintf(Afile_name, 32, "%s_%04d", g_CAfile_prefix, g_CAfile_index);
	Assert(pack_file(REFLECT_LIST_AFILE_DIR, 
					Afile_name, 
					REFLECT_LIST_AFILE_DIR,	
					Afile_name) == 0);
	/* 更新数据库的状态 */
	Assert(update_Afile_status(g_CAfile_timestamp));
	//Only store Afile.tgz
	rm_file(REFLECT_LIST_AFILE_DIR, Afile_name);
}

static int check_and_create_Afile()
{
	/*
	 *满足以下条件创建A文件:
	 *g_CAfile_fp == NULL 只有程序启动,第一次创建A文件时才会出现这种情况.
	 *距上次创建A文件超过5分钟.
	 *B文件重新创建了.
	 * */
	if (g_CAfile_fp == NULL) {
		/* 程序启动后,首次创建A文件 */	
		make_and_open_Afile();
		Assert(g_CAfile_fp);
		return 1;
	}

	if (g_CAfile_fp) {
	/* 检查首次创建A文件时间,是否超过5分钟 */
		if (pass_timestamp(g_CAfile_timestamp) >= AFILE_TIME_INTERVAL) {
			/* 关闭老文件 */
			close_old_Afile();
			make_and_open_Afile();
			Assert(g_CAfile_fp);
			return 2;
		}
	}

	if (is_Bfile_update(g_CAfile_prefix)) {
		close_old_Afile();
		make_and_open_Afile();
		Assert(g_CAfile_fp);
		return 3;
	}

	return 0;
}

void get_Afile_prefix_from_Bfile(char *Afile_prefix, 
								const char *Bfile)
{
	//Bfile B20140101
	//Afile_prefix A20140101
	strncpy(Afile_prefix, Bfile, APPEND_PREFIX_LEN);
	Afile_prefix[0] = 'A';
}

//TODO add write_lock for multi threads writing.
void append_to_Afile(string &data)
{
	check_and_create_Afile();
	Assert(g_CAfile_fp);
	//TODO deal the end-of-file error.
	/* 可能正在写的文件,人为的删除 */
	Assert(fprintf(g_CAfile_fp, "%s\n", data.c_str()) >= 0);
	fflush(g_CAfile_fp);

	create_Bfile();
}

int is_complete_Afile()
{
    /*检查程序A文件相关处理逻辑是否异常:
     *1:数据库中记录的A文件,磁盘中是否都存在.
     *2:记录A文件全局的index g_CAfile_index变量是否异常.
     *return 1A文件完整 0A文件不完整 -1检查时出现异常
     * */

	/* 从数据库中获取A文件名字 */
	MYSQL con;
	if (!get_instance(&con)) {
		LOG_ERRO("is_complete_Afile get_instance fail.");
		return -1;
	}

	/* FIXME 不使用全局的g_CAfile_prefix,有可能在检查的时候,
		添加A数据的线程,正好去创建一个新的A文件,且g_CAfile_prefix改变.
		磁盘中的A文件会被删除.导致校验不对*/
	char Afile_prefix[32] = {0};
	get_newest_Afile_prefix_from_Bfile(Afile_prefix);

	/* 从数据库中读取数据处于完成状态的A文件 */
	string err_str;
	char sql[MAX_SQL_SIZE] = {0};
	snprintf(sql, MAX_SQL_SIZE, 
				"SELECT * FROM %s.%s WHERE prefix = '%s' and status = 1;", 
				DB_NAME, AFILE_TABLE, Afile_prefix);
    //LOG_DEBU("Sql%s\n", sql);
	if (!mydb_operations::exec_sql(sql, err_str, &con)) {
		LOG_ERRO("[check Afile] exec_sql failed: %d: %s, %s", 
					mysql_errno(&con), mysql_error(&con), sql);
		destroy_connect(&con);
		return -1;
	}

	MYSQL_RES *res = mysql_store_result(&con);
	if (res == NULL) {
		LOG_ERRO("[check] mysql_use_result failed: %d: %s",
					mysql_errno(&con), mysql_error(&con));
		destroy_connect(&con);
		return -1;
	}

	/* 检查A文件的个数是否正常 */
	int files = (int)mysql_num_rows(res);
	/* 当files为0,存在g_CAfile_prefix不是最新的情况 */
	if (files == 0) {
		mysql_free_result(res);
		destroy_connect(&con);
		return 1;
	}

    /* 此处检查g_CAfile_prefix是否异常.在一些人为的情况下会出现该情况 */
	if ((files < (g_CAfile_index - 1)) || (files > g_CAfile_index)) {/* 最后一个文件可能处于处于未完成状态 */
		mysql_free_result(res);
		destroy_connect(&con);
		return 0;
	}

	/* 检查数据库中记录的A文件,在磁盘中是否存在 */
	MYSQL_ROW row;
	char path[1024] = {0};
	while ((row = mysql_fetch_row(res)) != NULL) {
        //LOG_DEBU("ROW:%s\n", row[0]);
		joint_path_by_dir_and_name(path, REFLECT_LIST_AFILE_DIR, row[0]);
		Assert(strlen(path) + strlen(".tgz") < 1024);
		strcat(path, ".tgz");
        //LOG_DEBU("Path:%s\n", path);
		if (!is_file_exist(path)) {
			mysql_free_result(res);
			destroy_connect(&con);
			LOG_WARN("check afile isn't compelet. [%s] is not exits.", path);
			return 0;
		}
	}

	mysql_free_result(res);
	destroy_connect(&con);
	return 1;
}


