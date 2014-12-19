#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <map>
#include <string>
#include <dirent.h>

#include "common.h"
#include "global.h"
#include "config.h"
#include "host_dump.h"
#include "slib/path.h"
#include "slib/utils.h"
#include "slib/json.h"
#include "slib/xxtea.h"
#include "sw_cond_mutex.h"
#include "mysql_worker.h"
#include "file_opt.h"

using namespace std;
//using namespace boost;
using namespace slib;

SMutex g_Bfile_mutex;
string g_Bfile;/* 最新的B文件名 */
int g_Bfile_index;/* 最新的文件index */

void send_create_Bfile_signal() {
	if (create_Bfile_lock->trylock()) {/*防止重复创建B,应该可以去掉*/
	    g_cond_mutex->send_signal();/* 创建B文件条件变量 */
		//g_cond_mutex->lock();
		//g_cond_mutex->unlock();
		LOG_WARN("new month create Bfile. send cond.");
		create_Bfile_lock->unlock();
	} else {
		LOG_WARN("new month create Bfile, but create Bfile busy.");
	}
}

void read_globle_Bfile(string &Bname, int &Bindex)
{
	g_Bfile_mutex.lock();
	Bname = g_Bfile;
	Bindex = g_Bfile_index;
	g_Bfile_mutex.unlock();
}

void set_globle_Bfile(const char *Bname, int Bindex)
{
/* 防止别的线程在读的时候,写入值 */
	g_Bfile_mutex.lock();
	g_Bfile = Bname;
	g_Bfile_index = Bindex;
	g_Bfile_mutex.unlock();
}

bool static is_valid_basic_file(const char *name)
{
	Assert(name);
	/* B文件名格式 B+年+月+2编.tgz */
	if (BASIC_NAME_FLAG != *name)
		return false;
	if (BASIC_NAME_LEN + strlen(".tgz") != strlen(name))
		return false;

	for(int i = 1; i < BASIC_NAME_LEN; i++) {
		if (!isdigit(*(name + i)))
			return false;
	}
	const char *p = name + strlen(name) - 4;
	if (strcmp(p, ".tgz") != 0) {
		return false;
	}
	/*FIXME 没有做更加严格的检查,
	 * 比如year < 2000 or > 2050;mon > 12 or < 1*/
	return true ;
}

static void make_Bfile_prefix_by_cur_date(char *Bfile_prefix)
{
	Assert(Bfile_prefix);
	struct tm tm_s;
	time_t now = time(NULL);
	char date[BASIC_PREFIX_LEN + 1] = {0};
	strftime(date, BASIC_PREFIX_LEN, "%Y%m", localtime_r(&now, &tm_s));
	/* B文件格式 B + 年 + 月 + 编号 */
	memset(Bfile_prefix, 0, BASIC_PREFIX_LEN);
	Bfile_prefix[0] = 'B';
	strcat(Bfile_prefix, date);
}

static bool check_necessary_dir()
{
	char cmd[512] = {0};
	if (!is_file_exist(REFLECT_LIST_AFILE_DIR)) {
		snprintf(cmd, 512, "mkdir -p %s", REFLECT_LIST_AFILE_DIR);
		Assert(system(cmd) != -1);
	}

	if (!is_file_exist(REFLECT_LIST_BASIC_DIR)) {
		snprintf(cmd, 512, "mkdir -p %s", REFLECT_LIST_BASIC_DIR);
		Assert(system(cmd) != -1);
	}

	if (!is_file_exist(REFLECTOR_UPDATE_DIR)) {
		snprintf(cmd, 512, "mkdir -p %s", REFLECTOR_UPDATE_DIR);
		Assert(system(cmd) != -1);
	}
	return true;
}

/* 本月的B文件可能不存在或存在多个 */
bool get_cur_month_newest_Bfile(string &Bname, char *Bfile_path)
{
	Assert(Bfile_path);
    struct dirent *next;

    DIR *dir = opendir(Bfile_path);
    if (!dir) {
		Perror("opendir");
		LOG_ERRO("Open %s :%s", Bfile_path, strerror(errno));
		//Assert(dir);
		check_necessary_dir();
	    return false;
   	}

	Bname = "";
	/* 通过月份,年份生成B文件前缀 */
	char Bfile_prefix[BASIC_PREFIX_LEN + 1] = {0};
	make_Bfile_prefix_by_cur_date(Bfile_prefix);

	/* FIXME 使用c++方式处理 */
	char tmp_name[BASIC_NAME_LEN + 1] = {0};
	/* 遍历目录,获取最新的B文件 */
	int max_Bfile_index = -1, Bfile_index = 0;
    while ((next = readdir(dir)) != NULL) {
		//B20140401.tgz
        if (!is_valid_basic_file(next->d_name))
			continue;
		//B20140401.tgz -> B20140401
		strncpy(tmp_name, next->d_name, BASIC_NAME_LEN);
		/* 是否为本月的B文件 */
		if (strncmp(Bfile_prefix, tmp_name, strlen(Bfile_prefix)) != 0)
			continue;
		Bfile_index = atoi(tmp_name + strlen(Bfile_prefix));
		if (Bfile_index > max_Bfile_index) {
			Bname = tmp_name;
			max_Bfile_index = Bfile_index;
		}
    }

	closedir(dir);
	if (Bname == "")  {
		return false;
	}
	return true;
}

static void rm_old_Bfile(char *Bfile_dir)
{
	Assert(Bfile_dir);
    DIR *dir = opendir(Bfile_dir);
    if (!dir) {
		Perror("opendir");
		Assert(dir);
		return;
   	}
#if 0
	/* 通过当的月份,年份生成B文件前缀 */
	char Bfile[16] = {0};
	make_Bfile_prefix_by_cur_date(Bfile);
	int cur_date = atoi(Bfile + 1);

	int date = 0;
    struct dirent *next;
	char Bfile_prefix[BASIC_PREFIX_LEN + 1] = {0};
	/* 遍历目录,获取最新的B文件 */
    while ((next = readdir(dir)) != NULL) {
        if (!is_valid_basic_file(next->d_name))
			continue;
		/* 是否为本月之前的B文件. Bname:B20140401.tgz */
		strncpy(Bfile_prefix, next->d_name + 1, BASIC_PREFIX_LEN - 1);
		date = atoi(Bfile_prefix);
		if (date < cur_date) {
			rm_file(Bfile_dir, next->d_name);
		}
    }
#endif
	/* 只保留最新B文件 */
	string newest_Bfile;
	int newest_Bfile_index;
	read_globle_Bfile(newest_Bfile, newest_Bfile_index);
    newest_Bfile += ".tgz";

	string Bfile;
    struct dirent *next;
	/* 遍历目录,获取最新的B文件 */
    while ((next = readdir(dir)) != NULL) {
        if (!is_valid_basic_file(next->d_name))
			continue;
		Bfile = next->d_name;
        /*TODO 当B文件index号由大变小,需要删除,比自己大的号 */
		if (newest_Bfile.compare(Bfile) != 0) {
			LOG_INFO("NewFile:%s Delete Bfile :%s",
                        newest_Bfile.c_str(), next->d_name);
			rm_file(Bfile_dir, next->d_name);
		}
    }
	closedir(dir);
}

int make_Bfile(const char *Bfile_prefix,
				int Bfile_index,
				const char *dir,
				string &Bfile_name)
{
	Assert(dir && Bfile_prefix);
	Assert(Bfile_index > 0);

	char temp_Bfile[16] = {0};
	snprintf(temp_Bfile, 16, "%s%02d", Bfile_prefix, Bfile_index);
	char path[512] = {0};
	joint_path_by_dir_and_name(path, dir, temp_Bfile);

	/* FIXME 错误处理 */
	Assert(touch_afile(path) == 0);
	Bfile_name = temp_Bfile;
	return 0;
}

int read_DB_write_to_Bfile(char *path, char *tb_name)
{
	MYSQL con;
	if(!get_instance(&con)) {
		return -1;
	}
	/* 从数据库中读取数据 */
	string err_str;
	char sql[MAX_SQL_SIZE] = {0};
	snprintf(sql, MAX_SQL_SIZE, 
			"SELECT DISTINCT wurl,curl FROM %s.%s;", 
			DB_NAME, tb_name);
	if(!mydb_operations::exec_sql(sql, err_str, &con)) {
		LOG_ERRO("[do_dump] exec_sql failed: %d: %s, %s", 
					mysql_errno(&con), mysql_error(&con), sql);
		destroy_connect(&con);
		return -1;
	}

	MYSQL_RES *res = mysql_use_result(&con);
	//MYSQL_RES *res = mysql_store_result(&con);
	if (res == NULL) {
		LOG_ERRO("[do_dump] mysql_use_result failed: %d: %s", 
					mysql_errno(&con), mysql_error(&con));
		destroy_connect(&con);
		return -1;
	}

	/* 打开文件 */
	FILE *fp = fopen(path, "w");
	if (fp == NULL) {
		Perror(path);
		Assert(fp);
		mysql_free_result(res);
		destroy_connect(&con);
		return -1;
	}

	/*TODO 通过sql命令直接写文件 */
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(res)) != NULL) {
		unsigned long *lens = mysql_fetch_lengths(res);
		string wurl(row[0], lens[0]);
		string curl(row[1], lens[1]);
		//cout << wurl << curl << endl;
		if (fprintf(fp, "%s %s\n", wurl.c_str(), curl.c_str()) < 0) {
			Perror("fprintf");
			LOG_ERRO("fprintf [%s] to [%s] failed.", wurl.c_str(), path);
			mysql_free_result(res);
			destroy_connect(&con);
			fclose(fp);
			return -1;
		}
	}
	LOG_INFO("fprintf [%s] end.", path);
	mysql_free_result(res);
	destroy_connect(&con);
	fclose(fp);
	return 0;
}

int create_Bfile(string &newest_Bfile, int &newest_Bfile_index)
{
	char Bfile_prefix[16] = {0};
	make_Bfile_prefix_by_cur_date(Bfile_prefix);
    LOG_INFO("%s", Bfile_prefix);

	int Bfile_index = 0;
	if (strncmp(newest_Bfile.c_str(),
				Bfile_prefix,
				BASIC_PREFIX_LEN) == 0) {
		/* 使用旧的B文件前缀,index加1 */
		Bfile_index = newest_Bfile_index + 1;
        /*TODO 到达99之后,重新从0开始, 此处理并不是很好*/
        if (Bfile_index == 100) {
            Bfile_index = 1;
            LOG_ERRO("B index arrive to 100!");
        }
	} else {
		/* 新的一月,B文件index从1开始 */
		Bfile_index = 1;
	}

	string Bfile_name;
	/* 创建B文件,如B20140401*/
	if (make_Bfile(Bfile_prefix, Bfile_index, REFLECT_LIST_BASIC_DIR, Bfile_name) != 0) {
		return -1;
	}

	char rm_cmd[512] = {0};
	char path[1024] = {0};
	joint_path_by_dir_and_name(path, REFLECT_LIST_BASIC_DIR, Bfile_name.c_str());
	if (read_DB_write_to_Bfile(path, DATA_TABLE) != 0) {
		snprintf(rm_cmd, 512, "rm -f %s", path);
		system(rm_cmd);
		return -1;
	}

	/* 保存MD5值 */
	string cmd = "md5sum ";
	cmd += path;
	cmd += " > ";
	cmd += REFLECT_LIST_BASIC_MD5_TEMP;/* 先保存到临时文件中 */
	system(cmd.c_str());

	/* 生成压缩文件, TODO 处理失败情况,比如磁盘满,压缩失败. */
	Assert(pack_file(REFLECT_LIST_BASIC_DIR,
					Bfile_name.c_str(),
					REFLECT_LIST_BASIC_DIR,
					Bfile_name.c_str()) == 0);
    if (rename(REFLECT_LIST_BASIC_MD5_TEMP, REFLECT_LIST_BASIC_MD5) != 0) {
        LOG_ERRO("rename %s %s error!",
                REFLECT_LIST_BASIC_MD5_TEMP, REFLECT_LIST_BASIC_MD5);
    }

	newest_Bfile = Bfile_name;
	newest_Bfile_index = Bfile_index;
	/* 仅仅保存压缩文件 */
	rm_file(REFLECT_LIST_BASIC_DIR, Bfile_name.c_str());
	LOG_INFO("Create %s file.", Bfile_name.c_str());
	return 0;
}

void *create_Bfile_thread(void *arg)
{
	//mysql_thread_init();
	string newest_Bfile;
	int newest_Bfile_index = 0;

	/* TODO The program expectation, when wring Bfile. */
	/* 本月的B文件是否已创建, 没有则创建B文件 */
	if (!get_cur_month_newest_Bfile(newest_Bfile, REFLECT_LIST_BASIC_DIR)) {
		/* 创建B文件,并保存文件名和B文件index */
		if (create_Bfile(newest_Bfile, newest_Bfile_index) != 0) {
			LOG_ERRO("Create Bfile fail!");
			exit(0);
		}
	} else {
		newest_Bfile_index = atoi(newest_Bfile.c_str() + BASIC_PREFIX_LEN);
	}
	/* 更新全局的B文件和index */
	set_globle_Bfile(newest_Bfile.c_str(), newest_Bfile_index);
	/* 删除老的B文件 */
	rm_old_Bfile(REFLECT_LIST_BASIC_DIR);

	while (1) {
		/* 等待条件变量成立 */
		if (g_cond_mutex->lock_and_wait() != 0) {
			/* 阻塞被意外终止 */
			LOG_INFO("g_cond_mutex->unlock 0");
			g_cond_mutex->unlock();
			continue;
		}
        LOG_INFO("recv a Single.");
		if (create_Bfile(newest_Bfile, newest_Bfile_index) != 0) {
			g_cond_mutex->unlock();
			continue;
		}
		/* 更新全局的B文件和index */
		set_globle_Bfile(newest_Bfile.c_str(), newest_Bfile_index);
		LOG_INFO("g_cond_mutex->unlock 2");
		g_cond_mutex->unlock();
		/* 删除老的B文件 */
		rm_old_Bfile(REFLECT_LIST_BASIC_DIR);
	}
	//mysql_thread_end();
	return NULL;
}

#if 0
static void feed_alarm()
{
	struct tm tm_s;
	time_t now = time(NULL);
	localtime_r(&now, &tm_s);

	/* 计算下一月的年份和月份 */
	int mon = (tm_s.tm_mon + 1) % 12;
	int year = (mon == 0) ? tm_s.tm_year + 1 : tm_s.tm_year;

	/* 在下一月的1日1时触发 */
	struct tm trigger_tm;
	memset(&trigger_tm, 0, sizeof(trigger_tm));
	trigger_tm.tm_year = year;
	trigger_tm.tm_mon = mon;
	trigger_tm.tm_mday = 1;
	trigger_tm.tm_hour = 1;

	/* 计算时间差 */
	time_t diff_sec = mktime(&trigger_tm) - time(NULL);
	alarm(diff_sec);
}
#endif
//void *alarm_handle_thread(void *arg)
//{
//	string newest_Bfile;
//	while (1) {
#if 0
		feed_alarm();
		/* 等待定时触发 */
		if (g_pthread_signal->wait_signal() < 0) {
			continue;
		}
		/* 本月的B文件是否已创建, 否则创建B文件 */
		if (!get_cur_month_newest_Bfile(newest_Bfile, REFLECT_LIST_BASIC_DIR)) {
			/* 发送信号, 唤醒创建B文件线程 */
			g_cond_mutex->send_signal();
			LOG_INFO("Alarm send signal.");
		}
		LOG_INFO("Alarm trigger.");
#else
//		sleep(100);
#endif
//	}
//}

