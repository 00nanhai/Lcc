#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <boost/lexical_cast.hpp>
#include <err.h>
#include <time.h>
#include <errno.h>

#include "common.h"
#include "global.h"
#include "request.h"
#include "config.h"
#include "mysql_worker.h"
#include "slib/path.h"
#include "slib/utils.h"
#include "slib/json.h"
#include "slib/xxtea.h"
#include "host_dump.h"
#include "sw_pthread_signal.h"
#include "process_opt.h"
#include "create_A.h"
#include "create_B.h"
#include "response.h"
#include "evhttp_opt.h"
#include "file_opt.h"

#include <my_log.h>

using namespace std;
using namespace slib;

char DB_NAME[32];
char DATA_TABLE[32];
Config *g_base_conf;
vector<string> *g_tb_names;
PthreadSignal *g_pthread_signal;
sw_cond_mutex *g_cond_mutex;

int exiting = 0;
//const std::string SVN_VERSION = "$(VERSION_STR)";

/* 解析运行选项 */
void parse_parameter(char argc, char **argv, 
					bool &deamon, string &config_file)
{
	int opt;
	//printf("%s %s %s %s\n", PRO_VERSION, UPDATE_TIME, SVN_VERSION, COMPLIE_TIME);
	while((opt = getopt(argc, argv, "c:vVdh")) != -1){
		switch(opt){
			case 'c':/* 用户指定配置文件 */
				config_file = optarg;
				break;

			case 'v':
				//reflector  version:0.6.1  update:2013-12-17
				cout << LCC_PROCESS_NAME << "\tversion:";
				cout << PRO_VERSION << "\t" << "update:" << UPDATE_TIME << endl;
				exit(0);
				break;
			case 'd':
				deamon = true;
				break;
			case 'V':
				cout << "svn vsrsion:" << SVN_VERSION << "\t";
				cout << "complie time:" << COMPLIE_TIME << endl;
				exit(0);
			case 'h':
				cout << "-c		define the config file path default is " << DEFAULT_LCC_CONF << endl;
				cout << "-v		show lcc version info " << endl;
				cout << "-h		show this message" << endl;
				exit(0);
				break;
		}
	}
}

int	prase_config_file(Config *config_obj, string config_file)
{
	Assert(config_obj);
	LOG_INFO("Loading config file:%s",config_file.c_str());
	if (!config_obj->read(config_file)) {
		LOG_INFO("Invalid config file:%s",config_file.c_str());
		return -1;
	}
	return 0;
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

static bool init_db_table()
{
	g_tb_names = new vector<string>();
	g_tb_names->push_back(g_base_conf->get("tb_names").c_str());
	/*TODO*/
	const string db = g_base_conf->get("db_name");
	const string table_name = g_base_conf->get("tb_names");
	strncpy(DB_NAME, db.c_str(), 32);
	strncpy(DATA_TABLE, table_name.c_str(), 32);
	/*TODO 检查数据库或表是否存在,不存在创建 */
	return true;
}

int main(int argc, char *argv[])
{
	 //mysql_server_init();
	 //mysql_thread_init();
	//printf("%s %s %s %s\n", PRO_VERSION, UPDATE_TIME, SVN_VERSION, COMPLIE_TIME);
	struct event_base *evbase ;
	struct evhttp *httpd ;
	host_records host_manager;
	bool deamon = false;
	string base_config = DEFAULT_LCC_CONF;
	parse_parameter(argc, argv, deamon, base_config);
	//TODO
	if(deamon == true){
		pid_t _t = fork();
		if(_t < 0){
			cerr << "failed to run in deamon mode " << errno << endl;
			exit(1);
		}else if( _t > 0){
			return 0;
		}
	}

	/* 是否重复执行 */
    unsigned int repeat_pid =
				is_process_repeat_run(getpid(), LCC_PROCESS_NAME);
    if (repeat_pid != 0) {
		LOG_ERRO("A process named %s is running, pid is %d \n",
				LCC_PROCESS_NAME, repeat_pid);
		exit(1);
    }

	/* FIXME 当数据库中的表正在被枷锁的时候关掉连接,mysql需要重启.
		或许有更好的办法  */
	system("service mysqld restart >> /dev/null");

	/* 检查必要的目录是否存在 */
	check_necessary_dir();

	/* 解析配置文件 */
	g_base_conf = new Config();
	if (g_base_conf == NULL) {
		LOG_ERRO("Invalid base config file [%s]", base_config.c_str());
		exit(1);
	}
	if (prase_config_file(g_base_conf, base_config) != 0) {
		LOG_ERRO("Prase %s error.", base_config.c_str());
		exit(1);
	}

	/* 初始化日志项 */
	const string r_log_path = g_base_conf->get("r_log_path");
	const string w_log_path = g_base_conf->get("w_log_path");
	const string prefix = g_base_conf->get("prefix");
	int level = boost::lexical_cast<int>(g_base_conf->get("log_level"));
	xLog_init(r_log_path.c_str(), w_log_path.c_str(), prefix.c_str(), level);

	if (!init_response()) {
		LOG_ERRO("init response fail.");
		goto exit;
	}
	/* 初始化DB_NAME DATA_TABLE */
	init_db_table();

	/* 屏蔽信号集 */
	//g_pthread_signal = new PthreadSignal(SIGALRM);
	//Assert(g_pthread_signal);
	/* 初始化条件变量  */
	g_cond_mutex = new sw_cond_mutex;
	Assert(g_cond_mutex);

	/* 启动创建B文件线程 */
	pthread_t B_thread_id;
	pthread_create(&B_thread_id, NULL, create_Bfile_thread, NULL);

#if 1
	//TODO 以更好的方式处理
	// Set timer for dump data-records specialed by host
	if (!host_manager.start()) {
		LOG_ERRO("host_manager.start( ) Failed.");
		goto exit;
	}
#endif
	/* 初始化创建A文件相关的项 */
	/* TODO 等到B文件初始化完成后,才能初始化创建A相关的项 */
	sleep(50);
	init_Afile();
	/*FIXME 将libevent的处理封装一下 */
	if (!evhttp_init(&evbase, &httpd)) {
		LOG_ERRO("init evhttp fail.");
        return -1;
	}
	evhttp_base_dispath(evbase);
exit:
/*TODO 释放资源 */
	//mysql_thread_end();
	//mysql_server_end();
	return 1;
}

