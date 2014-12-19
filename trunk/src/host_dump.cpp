#include <sys/time.h>
#include <time.h>
#include <mysql/mysql.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>   
#include <stdio.h>
#include <sys/time.h>  
#include <errno.h> 
#include <unistd.h> 
#include <event.h>
#include <pthread.h>
#include <evhttp.h>
#include <event2/thread.h>
#include <errno.h>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "global.h"
#include "config.h"
#include "mydb_instance.h"
#include "mysql_worker.h"

#include "my_md5.h"
#include "host_dump.h"

using namespace std;
//const string  g_host_file_dir = "/var/log/xFenguang/host_dump";
const string  g_host_file_dir = "/usr/local/xFenguang/tmp/host_dump";

pthread_mutex_t host_records::host_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t host_records::loop_tid = 0 ;

static MYSQL *g_hostdump_db;
bool init_hostdump_sql()
{
	g_hostdump_db = new MYSQL();
	if (g_hostdump_db == NULL) {
		LOG_INFO("alloc mem for g_hostdump_db;");
		return false;
	}
	
	if (!get_instance(g_hostdump_db)) {
		LOG_INFO("get_instance for g_hostdump_db;");
		return false;	
	}
	return true;
}

static int get_cur_hour()
{
    time_t tt = time(NULL);
    tm* t= localtime(&tt);
    return t->tm_hour;
}

void* host_records::loop(void *arg)
{
    //mysql_thread_init();
    init_hostdump_sql();
#if 0
    while(true) {	
        host_records::dump( 0 ) ;
        LOG_INFO("host_dump thread  sleep(14400); ");
        sleep(14400);
    }
#else
    host_records::dump(0);/*程序每次启动都生成hostdump文件*/
    int flag = 1;/*标记当天是否已生成hostdump文件,每天在9:00-9:59只生成一次hostdump文件*/
    while(true) {
        int hour = get_cur_hour();
        if ((hour == 9) && (flag == 0)) {
            LOG_INFO("Create hostdump file... ");
            host_records::dump(0);
            flag = 1;
        }
        else if (hour > 9) {/*5点之后hostdump已经创建过文件*/
            flag = 0;
        }
        LOG_INFO("Wait %d = 9am, flag:%d... ", hour, flag);
        sleep(2000);
    }
#endif
    return NULL;
    //mysql_thread_end();
}

bool host_records::start()
{
	//init_hostdump_sql();
	string tmp_path = g_host_file_dir + "/tmp/" ;
	string dump_path = g_host_file_dir +"/" ;
	// Check the dir path is invalid 
	string mk_dir1 = "mkdir -p " + tmp_path ;
	string mk_dir2 = "mkdir -p " + dump_path ;
	LOG_INFO("%s \n %s", mk_dir1.c_str(), mk_dir2.c_str());
	system(mk_dir1.c_str());
 	system(mk_dir2.c_str());

	sleep(1);
	int ret = pthread_create(&host_records::loop_tid , NULL,host_records::loop , NULL);
	if(ret != 0)
	{
		LOG_ERRO("pthread_create host_records::loop( ) failed.");
		return false;
	}
	else
	{
		LOG_INFO("pthread_create host_records::loop( ) OK.");
		return true ;
	}
}

extern vector<string> *g_tb_names;
//EEIWANT need some change
void host_records::dump(int)
{
	static int  cnt = 0 ;
	cnt++ ;
	LOG_INFO("dump index : %d" ,  cnt);

	const string tmp_path = g_host_file_dir + "/tmp/";
	const string dump_path = g_host_file_dir;

	vector<string> datas;
	vector<string>::iterator iter;
	//remove old dump files in tmp path
	list_files(tmp_path, datas);
	iter = datas.begin();
	while(iter != datas.end()) {
		if(remove(iter->c_str())){
			LOG_ERRO("failed to remove:%s errmsg:%s", iter->c_str(), strerror(errno));
			goto exit;
		}
		iter++;
	}

	//dump data record to files
	iter = g_tb_names->begin();
	while(iter != g_tb_names->end()) {
		if(host_records::dump_tbdata_to_files(*iter) == false){
			goto exit;
		}
		iter++;
	}

	//tar files
	datas.clear();
	host_records::list_files(tmp_path, datas);	
	tar_dump_files(datas);

exit:
	return ;
}

bool host_records::dump_host_record(map<string, FILE*> &host_files, const string &host, const string &ckey, const string &curl, const string &size, const string &error,const string &tb_name)
{
	string tmp_path = g_host_file_dir + "/tmp/" + host;

	FILE* fp = NULL;
	if(host_files.find(host) == host_files.end())
	{
		fp = fopen(tmp_path.c_str(), "a");

		if(NULL == fp)
		{
			LOG_ERRO("open [%s] failed.", tmp_path.c_str());
			return false;
		}
		host_files[host] = fp;
	}
	else
	{
		fp = host_files[host];
	}

	if(0 > fprintf(fp, "%s^%s^%s^%s^%s\n", ckey.c_str(), curl.c_str(), size.c_str(), error.c_str(), tb_name.c_str()))
	{
		LOG_ERRO("fprintf [%s] to [%s] failed: %d: %s", ckey.c_str(), tmp_path.c_str(), errno, strerror(errno));
		return false;
	}

	return true;
}

void host_records::tar_dump_files(vector<string> &host_files_path)
{
	const string dump_path = g_host_file_dir + "/";
	const string tmp_path = dump_path + "tmp/";

	vector<string>::iterator it = host_files_path.begin();
	for( ; it != host_files_path.end(); ++it)
	{
		const string srcfile(*it);
		const string src_file_path = srcfile.substr(0, srcfile.rfind("/"));
		const string host = srcfile.substr(srcfile.rfind("/") + 1);
		const string tmp_tar_file = tmp_path + host + "_bak.tgz";
		string dst_tar_file = dump_path + host + ".tgz" ;
		string cmd = "tar -zcf " + tmp_tar_file + " -C" + src_file_path + " " + host +  " 1>>/dev/null 2>>/dev/null";
		LOG_INFO("[tar_dump_files] [%s]", cmd.c_str());
		system(cmd.c_str());
		string md5 = "";
		/* add md5 防止hostdump文件生成一半挂掉 */
		if (calc_md5(tmp_tar_file, md5)) {
			dst_tar_file += ".";
			dst_tar_file += md5;
		} else {
			cmd = "rm -f " + tmp_tar_file;
			system(cmd.c_str());
			continue;
		}
		cmd = "mv -f " + tmp_tar_file + " " + dst_tar_file + " 1>>/dev/null 2>>/dev/null";
		LOG_INFO("mv dump file:%s", cmd.c_str());
		// place the lock here to protect the dest file 
		string temp_cmd = "rm -f " + dump_path + host + ".tgz* " + dump_path + "tmp/" + host;
		pthread_mutex_lock(&host_mutex);
		system(temp_cmd.c_str());
		system(cmd.c_str());
        //printf("Cmd:%s TempCmd：%s\n", cmd.c_str(), temp_cmd.c_str());
		pthread_mutex_unlock(&host_mutex);
	}
}

bool
host_records::list_files(const string &path, vector<string> &files_path){
	DIR * dir = opendir(path.c_str());
	struct dirent *next;
	const string &base_name = (path[path.size() - 1] == '/') ? path: path + "/";
	bool ret = true;
	if(dir == NULL){
		ret = false;
		goto exit;
	}

	while((next = readdir(dir)) != NULL){
		if(next->d_type == DT_REG){
			const string f = base_name + next->d_name;
			files_path.push_back(f);
		}else{
			continue;
		}
	}

exit:
	if(dir != NULL) closedir(dir);
	return ret;
}

bool
host_records::dump_tbdata_to_files(const string &tb_name) {
	unsigned long long count = 0;
	MYSQL *con = g_hostdump_db;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	bool ret = true;
	map<string, FILE *> host_files;
	string err_str;
	//string sql = "SELECT DISTINCT ckey, curl, host, size, error FROM ";
	string sql = "select ckey, curl, host, size, error from ";
	sql += DB_NAME;
	sql += ".";
	sql += tb_name;
#if 0 /* 采用不友好的处理方式,没有去掉重复数据, group by 太慢啦 */
	sql += " group by curl;";
#else
	sql += " ;";
#endif
	if(!con)
	{
		LOG_ERRO("Get database instance failed in host_records::dump( )" );
		ret = false;
		goto exit;
	}
	if(!mydb_operations::exec_sql(sql.c_str(),err_str,con))
	{
		LOG_ERRO("Execute [%s] Failed : [%d : %s].",
				sql.c_str(),  mysql_errno( con ), mysql_error(con) );
		ret = false;
		goto exit;
	}
	LOG_INFO("hostdump sql:%s", sql.c_str());
	res = mysql_use_result( con );/*这里存在问题,没有搞定*/
	//res = mysql_store_result( con );
	if(NULL == res)
	{
		LOG_ERRO("[do_dump] mysql_use_result failed: %d: %s", 
					mysql_errno( con ), mysql_error(con));
		ret = false;
		goto exit;
	}
	
	while((row = mysql_fetch_row(res)) != NULL)
	{	
		count++;
		unsigned long *lens = mysql_fetch_lengths(res);
		string ckey(row[0], lens[0]);
		string curl(row[1], lens[1]);
		string host(row[2], lens[2]);
		string size(row[3], lens[3]);
		string error(row[4], lens[4]);

		if(!host_records::dump_host_record(host_files, host, ckey, curl, size, error, tb_name))
		{
			ret = false;
			goto exit;
		}
	}
	LOG_INFO("hostdump num:%llu", count);

exit:
	if(res != NULL)	mysql_free_result(res);

	map<string, FILE *>::iterator i = host_files.begin();
	while(i != host_files.end()){
	   	fclose(i->second);
		i++;
	}

	return ret;
}
