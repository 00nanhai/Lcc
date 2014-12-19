#ifndef _LCC_GLOBAL_H_
#define _LCC_GLOBAL_H_

#define SLIB_SLOG_HANDLE g_log

#include <string>
#include <map>
#include <pthread.h>
#include <event.h>
#include <evhttp.h>
//#include <mysql/mysql.h>
#include "slib/path.h"
#include "slib/utils.h"
#include "slib/json.h"
#include "slib/xxtea.h"
#include "smutex.h"
#include "config.h"

#include "sw_cond_mutex.h"
#include "sw_pthread_signal.h"
#include <my_log.h>

using namespace std; 
using namespace slib; 

#define HTTP_403 403
#define HTTP_302 302
#define HTTP_500 500
#define DEFAULT_DB_CHARSET "utf8"
#define LCC_PROCESS_NAME "lcc"

#define MAX_DIRNAME_LEN 1024
#define MAX_LINE_LEN 2048

#ifndef pid_t
#define pid_t unsigned int 
#endif

/* 数据库相关配置 */
extern char DB_NAME[32];
extern char DATA_TABLE[32];
//#define DB_NAME				"test"
//#define DATA_TABLE			"data"
#define MAX_SQL_SIZE		4096

/* A文件相关配置 */
#define AFILE_TABLE			"Afile_info"
#define APPEND_NAME_FLAG	'A'
#define APPEND_NAME_LEN		14 //strlen(A20140101_0001)
#define APPEND_PREFIX_LEN	9
#define AFILE_TIME_INTERVAL	300 //5 * 60s
#define REFLECT_LIST_AFILE_DIR "/data/log/xFenguang/lcc/Afile/"

/* B文件相关配置 */
#define BASIC_NAME_LEN	 9 //strlen(B20140101)
#define BASIC_PREFIX_LEN 7
#define BASIC_NAME_FLAG	'B'
#define REFLECT_LIST_BASIC_DIR "/data/log/xFenguang/lcc/Bfile/"
#define REFLECT_LIST_BASIC_MD5 "/data/log/xFenguang/lcc/Bfile/.Bfile.md5"
#define REFLECT_LIST_BASIC_MD5_TEMP "/data/log/xFenguang/lcc/Bfile/.Bfile.md5.tmp"

/* LCC处理的请求 */
enum
{
	GET_CKEY = 0,
	SET_CKEY,
	GET_CURL,
	SET_CURL,
	DEL_CURL,
	DUMP_HOST_RECORDS,
	CREATE_B_FILE,
#if 0	
	GET_CURRENT_MAX_B_AND_A_INDEX,
#endif
	GET_FILE,	
	GET_CKEY_BY_ID,	 /*20140818 by lzy 该方法废弃*/
	SET_CURL_HAVE_ID,/*20140818 by lzy 该方法废弃*/
	LCC_REQ_COUNT,
	CHECK_FILE,

/*20140818 by lzy 获取文件大小,用于回原探测工具,
  当保存资源文件大小和源站文件大小不一致时,
  认为源站资源已更新已保存的资源需要删除,
  资源删除由探测工具调用删除接口删除*/
    GET_RESOURCE_SIZE,
    DELAY_DEL,
    DELAY_DEL_REPORT,
    //...

    /*在该保留操作之前,添加新的操作*/
    LAST_RESERVE_OPT,
};

//#define REFLECT_LIST_DIR  "/data/log/xFenguang/lcc/"
enum db_conn_type
{
	SHORT_PROCESS_CONN = 0 ,
	HOSTDUMP_PROCESS_CONN  ,
	REFLECTORLIST_PROCESS_CONN  ,
	COUNTOF_PROCESS_CONN  ,
} ;

extern int exiting ;
extern XXTea   g_cryptor;
extern Config *g_base_conf;

extern const string  g_host_file_dir; 

/* 默认Lcc配置路径 */
#define DEFAULT_LCC_CONF "/usr/local/xFenguang/etc/lcc.conf"

/* 程序的版本号 */
const std::string VERSION = "0.8.2";

/* 用于创建B文件的条件变量 */
extern sw_cond_mutex *g_cond_mutex;

/* 用于定时创建B文件的线程信号 */
extern PthreadSignal *g_pthread_signal;

/* 全局最新的B文件和index */
extern string g_Bfile;
extern int g_Bfile_index;
extern SMutex g_Bfile_mutex;
extern SMutex *create_Bfile_lock;

/* 干扰文件列表名称 */
#define REFLECTOR_UPDATE_FILE "reflector_list_update.txt"

/* 干扰文件列表目录 */
#define REFLECTOR_UPDATE_DIR "/usr/local/xFenguang/lcc/"
#endif


