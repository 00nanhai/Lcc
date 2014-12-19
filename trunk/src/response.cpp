#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include <event.h>
#include <evhttp.h>
#include <my_log.h>

#include "common.h"
#include "global.h"
#include "slib/path.h"
#include "slib/utils.h"
#include "slib/json.h"
#include "slib/xxtea.h"
#include "smutex.h"
#include "file_opt.h"
#include "sw_cond_mutex.h"
#include "mydb_instance.h"

#include "my_md5.h"
#include "mysql_worker.h"
#include "host_dump.h"
#include "create_A.h"
#include "create_B.h"
#include "evhttp_opt.h"
#include "check_error.h"

using namespace std ;
using namespace slib ;
using namespace boost ;

MYSQL *g_response_con;
SMutex *create_Bfile_lock;
SMutex *set_curl_lock;
bool init_response()
{
	create_Bfile_lock = new SMutex();
	set_curl_lock = new SMutex();
	g_response_con = new MYSQL();
	if (g_response_con == NULL) {
		return false;
	}
	return get_instance(g_response_con);
}

void response_get_ckey(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	string result;
	if (!mydb_operations::get_ckey_from_wurl(params["wurl"],
											params["tb_name"],
											result,
											g_response_con)) {
		LOG_INFO("get_ckey_from_wurl failed , wurl=%s tb_name = %s",
				params["wurl"].c_str(), params["tb_name"].c_str());
	}
	//LOG_INFO("result:%s", result.c_str());
	evhttp_replyinfo(req, buf, result.c_str());
}

/*已废弃*/
void response_get_ckey_by_id(struct evhttp_request *req,
							struct evbuffer *buf,
							map<string,string> &params)
{
	string result;
	if( !mydb_operations::get_ckey_from_id(params["id"],
											params["tb_name"],
											result,
											g_response_con)) {
		LOG_INFO("get_ckey_from_id failed , id=%s tb_name = %s",
				params["id"].c_str(), params["tb_name"].c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}

void response_get_curl(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	string result;
	if (!mydb_operations::get_record_from_ckey(params["ckey"], 
											  params["tb_name"], 
											  result, 
											  g_response_con)) {
		LOG_INFO("get_record_from_ckey failed , ckey=%s tb_name = %s",
				params["ckey"].c_str(), params["tb_name"].c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}

void response_set_curl(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	//TODO for test  0514
	//check_youku_wurl(params["wurl"]);

	string result;
	bool inserted = mydb_operations::set_ckey_curl(
			params["ckey"], params["curl"], 
			params["host"], params["size"], 
			params["wurl"], params["tb_name"],
			result, g_response_con);
	if (inserted) {
		string data = "+" + params["wurl"] + " " 
					+ params["curl"].substr(0, params["curl"].find('^'));
		append_to_Afile(data);
		LOG_INFO("set_ckey_curl done %s %s %s %s", 
				params["wurl"].c_str(), 
				params["ckey"].c_str(),
				params["curl"].c_str(), 
				params["tb_name"].c_str());
	} else {
		LOG_ERRO("set_ckey_curl failed %s %s", 
				params["ckey"].c_str(), result.c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}
/*已废弃*/
void response_set_curl_have_id(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	//TODO for test  0514
	//check_youku_wurl(params["wurl"]);

	string result;
	bool inserted = mydb_operations::set_ckey_curl_have_id(
			params["ckey"], params["curl"], 
			params["host"], params["size"], 
			params["wurl"], params["id"],
			params["tb_name"], result, 
			g_response_con);
	if (inserted) {
		string data = "+" + params["wurl"] + " " 
					+ params["curl"].substr(0, params["curl"].find('^'));
		append_to_Afile(data);
		LOG_INFO("set_ckey_curl_have_id done %s %s %s %s %s", 
				params["wurl"].c_str(), 
				params["ckey"].c_str(),
				params["curl"].c_str(), 
				params["tb_name"].c_str(),
				params["id"].c_str());
	} else {
		LOG_ERRO("set_ckey_curl_have_id failed ckey:%s wurl:%s %s", 
				params["ckey"].c_str(), params["wurl"].c_str(), result.c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}

void response_get_resource_size(struct evhttp_request *req,
                       struct evbuffer *buf,
					   map<string,string> &params)
{
	string result;
	if (mydb_operations::get_resource_size_by_curl(params["curl"], 
									params["tb_name"], 
									result, 
									g_response_con)) {
	} else {
		LOG_WARN("get_resource_size failed %s %s %s", 
				params["curl"].c_str(),
				params["tb_name"].c_str(), 
				result.c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}

void response_del_curl(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	string result;
	vector <string> deleted_wurls;
	if (mydb_operations::del_curl(params["curl"], 
									params["tb_name"], 
									deleted_wurls, 
									result, 
									g_response_con)) {
		vector <string>::iterator it;
		for (it = deleted_wurls.begin(); it != deleted_wurls.end(); it++) {
			string data = "-" + *it;
			append_to_Afile(data);
			LOG_INFO("delete_curl done %s %s", 
					params["curl"].c_str(), params["tb_name"].c_str());
		}
	} else {
		LOG_WARN("delete_curl failed %s %s %s", 
				params["curl"].c_str(),
				params["tb_name"].c_str(), 
				result.c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}

void response_delay_del(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	string result;
	vector <string> deleted_wurls;
	if (mydb_operations::delay_del(params["host"],
									params["tb_name"],
									deleted_wurls,
									result,
									g_response_con)) {
		vector <string>::iterator it;
		for (it = deleted_wurls.begin(); it != deleted_wurls.end(); it++) {
			string data = "-" + *it;
			append_to_Afile(data);
			LOG_DEBU("Delay del done %s %s %s",
					params["hsot"].c_str(), params["tb_name"].c_str(), (*it).c_str());
		}
	} else {
		LOG_WARN("Delay del failed %s %s %s",
				params["host"].c_str(),
				params["tb_name"].c_str(),
				result.c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}

void response_delay_del_report(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	string result;
	if (mydb_operations::delay_del_report(params["curl"],
                                    params["host"],
									params["tb_name"],
									result,
									g_response_con)) {
        LOG_INFO("Delay delete report curl done %s %s %s",
                params["host"].c_str(), params["curl"].c_str(), params["tb_name"].c_str());
	} else {
		LOG_WARN("Delay delete report curl failed %s %s %s %s",
                params["host"].c_str(),
				params["curl"].c_str(),
				params["tb_name"].c_str(),
				result.c_str());
	}
	evhttp_replyinfo(req, buf, result.c_str());
}

void response_create_Bfile(struct evhttp_request *req,
                          struct evbuffer *buf,
                          map<string,string> &params)
{
	int Bindex;
	string Bname;
	read_globle_Bfile(Bname, Bindex);
	/* 向B文件创建线程,发送信号 */
	g_cond_mutex->send_signal();

	evhttp_replyinfo(req, buf, "starting...");
	int msec = 10 * 60 * 1000;
	/* 等待B文件创建, 等待10分钟 */
	int ret = g_cond_mutex->lock((time_t)msec);/*TODO BUG, 超时不起作用*/
	LOG_INFO("ret :%d %d %d", ret, msec, ETIMEDOUT);
	if (ret == 0) {
		g_cond_mutex->unlock();
		LOG_INFO("Create Bfile sucess.");
	}
	else if (ret == ETIMEDOUT) {
		LOG_ERRO("Create Bfile timeout.");
	} else {
		LOG_ERRO("Create Bfile exception");
	}
}

bool create_reflect_list(char *path)
{
	int Bindex;
	string Bname;
	/* 获取程序目前记录的B文件index */
	read_globle_Bfile(Bname, Bindex);
	if (Bname == "") {
		LOG_ERRO("read_globle_Bfile fail.");
		return false;
	}

	/* 打开B的MD5值文件 */
	int tmp = open(REFLECT_LIST_BASIC_MD5, O_RDONLY, 0666);
	if (tmp < 0) {
		LOG_ERRO("Open %s fail.", REFLECT_LIST_BASIC_MD5);
		Perror(REFLECT_LIST_BASIC_MD5);
		//return false;
	}

	/* 读取B的MD5值文件 */
	char buf[256] = {0};
	if (read(tmp, buf, 254) < 32) {
		LOG_ERRO("read %s fail. [%s]", REFLECT_LIST_BASIC_MD5, buf);
		close(tmp);
		//return false;
	} else {
		close(tmp);
	}

	/* 干扰文件中MD5若不能成功校验,则不写md5值,
		download refelect_list模块重新生成B文件. */
	char md5[64] = {0};
	char filename[BASIC_NAME_LEN + 1] = {0};
	char *p = strchr(buf, ' ');
	if (p) {
		int len = p - buf;
		if (len != 32) {/*md5长为32*/
			LOG_ERRO("read %s fail. [%s]", REFLECT_LIST_BASIC_MD5, buf);
			//return false;
		}
		strncpy(md5, buf, len);
		p = strrchr(buf, '/');/*截断路径,只取B文件的名字*/
		if (p) {
			strncpy(filename, p + 1, BASIC_NAME_LEN);
		} else {
			memset(md5, '0', 64);
			LOG_ERRO("read %s fail. [%s]", REFLECT_LIST_BASIC_MD5, buf);
			//return false;
		}
	} else {
		memset(md5, 0, 64);
		LOG_ERRO("read %s fail. [%s]", REFLECT_LIST_BASIC_MD5, buf);
		//return false;
	}

	/* 比对MD5值保存的文件名和程序中保存的文件名 */
	if (strcmp(Bname.c_str(), filename) != 0) {
		memset(md5, 0, 64);
		LOG_ERRO("Bfilename [%s] != [%s]", Bname.c_str(), filename);
		//return false;
	}

	string Bfile_info = Bname + " ";
	Bfile_info += md5;
	Bfile_info += "\n";

	int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0666);
	if (fd < 0) {
		LOG_ERRO("Open %s", path);
		Perror(path);
		return false;
	}

	/* 将B文件的信息写入到update_list中 */
	if (write(fd, Bfile_info.c_str(), Bfile_info.length())
				!= (int)Bfile_info.length()) {
		LOG_ERRO("write [%s] [%s] fail.", path, Bfile_info.c_str());
		Perror(path);
		return false;
	}
	//write(fd, "\n", 1);

	char Afile_prefix[16] = {0};
	get_Afile_prefix_from_Bfile(Afile_prefix, Bname.c_str());

	/* 从数据库中读取该B文件对应的A文件 */
	char sql[MAX_SQL_SIZE] = {0};
	snprintf(sql, MAX_SQL_SIZE,
			"select name from %s.%s where prefix = '%s' and status = 1;",
			DB_NAME, AFILE_TABLE, Afile_prefix);

	string exec_ret;
	mydb_operations::exec_sql(sql, exec_ret, g_response_con);
	MYSQL_RES *res = mysql_use_result(g_response_con);
	if (res == NULL) {
		close(fd);
		return false;
	}
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(res)) != NULL) {
		unsigned long *lens = mysql_fetch_lengths(res);
		/* 处理写入出错信息 */
		write(fd, row[0], lens[0]);
		write(fd, "\n", 1);
	}
	mysql_free_result(res);
	close(fd);
	return true;
}

void response_get_file(struct evhttp_request *req,
                       struct evbuffer *buf,
					   map<string,string> &params)
{
	string file =  params["name"];
	char path[1024] = {0};
	if (*(file.c_str()) == APPEND_NAME_FLAG) {
		joint_path_by_dir_and_name(path, REFLECT_LIST_AFILE_DIR, file.c_str());
		Assert(strlen(path) + strlen(".tgz") < 1024);
		strcat(path, ".tgz");
		/* 检查该文件是否存在 */
		if (!is_file_exist(path)) {
			evhttp_send_failinfo(req, 404, "Not Found.");
			return;
		}
		evhttp_send_file(req, buf, path);
	}
	else if (*(file.c_str()) == BASIC_NAME_FLAG) {
		joint_path_by_dir_and_name(path, REFLECT_LIST_BASIC_DIR, file.c_str());
		Assert(strlen(path) + strlen(".tgz") < 1024);
		strcat(path, ".tgz");
		/* 检查该文件是否存在 */
		if (!is_file_exist(path)) {
			evhttp_send_failinfo(req, 404, "Not Found.");
			return;
		}
		evhttp_send_file(req, buf, path);
	}
	else if (file == REFLECTOR_UPDATE_FILE) {
		char path[1024] = {0};
		joint_path_by_dir_and_name(path,
				REFLECTOR_UPDATE_DIR,
				REFLECTOR_UPDATE_FILE);

		create_reflect_list(path);
		//FIXME add lock
		//if (!create_reflect_list(path)) {
		//	string reason = "create_reflect_list fail!";
		//	evhttp_send_failinfo(req, SW_HTTP_UNDEFILE_ERRNO, reason.c_str());
		//}
		evhttp_send_file(req, buf, path);
	} else {
		string reason = "Get file form error:" + file;
		evhttp_send_failinfo(req, SW_HTTP_CLIENT_ERRNO, reason.c_str());
	}
}

void search_hostdump_file(string path, string &file, string &key)
{
	file.clear();
	DIR *dir = opendir(path.c_str());
	if (!dir) {
		LOG_ERRO("search_hostdump_file: %s %s", file.c_str(), strerror(errno));
		return;
	}

	string name, md5, temp_file;
	std::size_t pos;
	struct dirent *next;
	while ((next = readdir(dir)) != NULL) {
		if (key.compare(0, key.length(), next->d_name, key.length()) != 0) {
			continue;
		}
		name = next->d_name;
		pos = name.rfind('.');
		if (pos == string::npos) {
			continue;
		}
		md5 = name.substr(pos + 1);
		if (md5.length() != 32) {
			continue;
		}
		temp_file = path + name;
		if (!check_md5(temp_file, md5)) {
			continue;
		}
		file = temp_file;
		break;
	}
}

void response_host_dump(struct evhttp_request *req,
                       struct evbuffer *buf,
                       map<string,string> &params)
{
	/*TODO temporary add for test */
	string dump_path = g_host_file_dir + "/";
	string prefix = params["host"] + ".tgz";
	//string path = dump_path + "/" + host + ".tgz";
	
	string file;
	file.clear();
	search_hostdump_file(dump_path, file, prefix);
	if (file.empty()) {
		/* send 404 */
		evhttp_send_failinfo(req, 404, "Not Found.");
		return;
	}
	pthread_mutex_lock(&host_records::host_mutex);
	evhttp_send_file(req, buf, file.c_str());
	pthread_mutex_unlock(&host_records::host_mutex);

	LOG_DEBU("trans file : %s" , file.c_str());
}

void response_check_file(struct evhttp_request *req,
							struct evbuffer *buf,
							map<string, string> &params)
{
	int Bindex;
	string Bname;
	/* 获取程序目前记录的B文件index */
	read_globle_Bfile(Bname, Bindex);
	if (Bname == "") {
		LOG_ERRO("read_globle_Bfile fail.");
		evhttp_replyinfo(req, buf, "EXCEPTION");
		return;
	}
    /*检查B文件是否存在*/
	char path[1024] = {0};
	Bname += ".tgz";
	joint_path_by_dir_and_name(path, REFLECT_LIST_BASIC_DIR, Bname.c_str());
	if (!is_file_exist(path)) {
		LOG_ERRO("[%s] is not exist.", path);
		evhttp_replyinfo(req, buf, "FAIL");
		return;
	}
    /*检查A文件*/
	int ret = is_complete_Afile();
	if (ret == 1) {
		evhttp_replyinfo(req, buf, "OK");
	} if (ret == 0) {
		evhttp_replyinfo(req, buf, "FAIL");
	} else {
		evhttp_replyinfo(req, buf, "EXCEPTION");
	}
}

//int g_response = 0;
void response(struct evhttp_request *req,
			  struct evbuffer *buf,
			  map<string,string> &params,
			  int opt_code)
{
	//g_response++;
	//for test 0514
	//string temp_wurl = params["wurl"];
	//if (temp_wurl.length() > 0) {
		//check_youku_wurl(temp_wurl);
	//}

	switch(opt_code)
	{
		case GET_CKEY :
			response_get_ckey(req, buf, params);
			break;
		case SET_CKEY :
			LOG_WARN("No support SET_CKEY.");
			evhttp_replyinfo(req, buf, "No support SET_CKEY.");
			break;
		case GET_CURL :
			response_get_curl(req, buf, params);
			break;
		case SET_CURL :
			set_curl_lock->lock();
			response_set_curl(req, buf, params);
			set_curl_lock->unlock();
			break;
		case DEL_CURL :
			response_del_curl(req, buf, params);
			break;
		case DUMP_HOST_RECORDS :
			response_host_dump(req, buf, params);
			break;
		case CREATE_B_FILE :
			//create_Bfile_lock->lock();
			if (create_Bfile_lock->trylock()) {
				response_create_Bfile(req, buf, params);
				LOG_INFO("response_create_Bfile");
				create_Bfile_lock->unlock();
			} else {
				LOG_INFO("busy");
				evhttp_replyinfo(req, buf, "busy");
			}
			break;
		case GET_FILE :
			response_get_file(req, buf, params);
			break;
		case GET_CKEY_BY_ID:/*已废弃*/
			response_get_ckey_by_id(req, buf, params);
			break;
		case SET_CURL_HAVE_ID:/*已废弃*/
			set_curl_lock->lock();
			response_set_curl_have_id(req, buf, params);
			set_curl_lock->unlock();
			break;
		case CHECK_FILE:
			response_check_file(req, buf, params);
			break;
        case GET_RESOURCE_SIZE:
            response_get_resource_size(req, buf, params);
			break;
        case DELAY_DEL:
            response_delay_del(req, buf, params);
            break;
        case DELAY_DEL_REPORT:
            response_delay_del_report(req, buf, params);
            break;
		default :
			LOG_ERRO("Request operate exception");
			//Assert(0);
	}
}

