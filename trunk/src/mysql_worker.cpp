#include <stdio.h>
#include <iostream>
#include <mysql/mysql.h>
#include "global.h"

#include "mydb_instance.h"
#include "mysql_worker.h"
#include "check_error.h"

using namespace std;

static string& replace_all_distinct(string& str, const string& old_value, const string& new_value)
{   
    for(string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
        if ((pos=str.find(old_value,pos)) != string::npos) {
            str.replace(pos,old_value.length(),new_value);
        } else {
            break;
        }   
    }   
    return str;
}

static string& adjust_wurl_for_sql(string wurl)
{
	LOG_INFO("wrul:%s", wurl.c_str());
	
	//"\'" -> "\\'"
	string old_str = "\\";
	string new_str = "\\\\";
	string temp_str = replace_all_distinct(wurl, old_str, new_str);
	//"'" -> "\'"
	old_str = "'";
	new_str = "\\'";
    return replace_all_distinct(wurl, old_str, new_str);
}

bool mydb_operations::exec_sql(const char *sql,
								string &exec_ret, 
								MYSQL *in_con = NULL) 
{
	MYSQL *con = in_con;
	int ret;
	int free_mysql = 0;
	if (con == NULL) {
		LOG_DEBU("Create a new connect.");
	/* 关闭或释放MYSQL */
		if ((con = new MYSQL()) == NULL) {
			LOG_ERRO("new MYSQL fail!");
			return false;
		}
		free_mysql = 1;
		if (!get_instance(con)) {
			goto END;
		}
	}

	LOG_DEBU("exec sql : %s", sql);
	ret = mysql_ping(con) ;
	if (ret) {
		LOG_ERRO("mysql_ping failed return %d: ecode = %d, ermsg = %s", 
					ret, mysql_errno(con), mysql_error(con));
		goto END;
	}

	if (mysql_query(con, sql)) {
		LOG_ERRO("mysql_query [%s] failed: %d: %s", 
				sql, mysql_errno(con), mysql_error(con));
		goto END;
	}

	if (free_mysql) {
		mysql_close(con);
		delete con;
	}
	return true;
END:
	if (free_mysql) {
		mysql_close(con);
		delete con;
	}
	return false;
}

bool mydb_operations::get_ckey_from_wurl(const string &wurl, 
										const string &tb_name, 
										string &ckey,
										MYSQL *con)
{

	string wurl_new = adjust_wurl_for_sql(wurl);
	//TODO for test  0514
	//check_youku_wurl(wurl_new);

	string sql = "SELECT ckey FROM `" + tb_name 
					+ "` WHERE wurl = '" + wurl_new + "' LIMIT 1" + ";";
	string k = "NULL";
	
	if (!mydb_operations::exec_sql(sql.c_str(), k, con)) {
		ckey = "FAIL";
		return false;
	}

	MYSQL_RES *res = mysql_use_result(con);
	if (res == NULL) {
		LOG_ERRO("mysql_use_result. [%s] failed: %d: %s", sql.c_str(),
			mysql_errno(con), mysql_error(con));
		ckey = "FAIL";
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(row == NULL) {
		LOG_WARN("Not found ckey. wurl:%s", wurl.c_str());
		mysql_free_result(res);
		ckey = "NULL";
		return false;
	}

	ckey = row[0];
	mysql_free_result(res);
	return true;
}

/*已废弃*/
bool mydb_operations::get_ckey_from_id(const string &id, 
										const string &tb_name, 
										string &ckey,
										MYSQL *con)
{
	string sql = "SELECT ckey FROM `" + tb_name 
					+ "` WHERE id = '" + id + "';";
	string k = "NULL";
	
	if (!mydb_operations::exec_sql(sql.c_str(), k, con)) {
		ckey = "FAIL";
		return false;
	}

	MYSQL_RES *res = mysql_use_result(con);
	if (res == NULL) {
		LOG_ERRO("mysql_use_result. [%s] failed: %d: %s", sql.c_str(),
			mysql_errno(con), mysql_error(con));
		ckey = "FAIL";
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(row == NULL) {
		mysql_free_result(res);
		LOG_WARN("Not found ckey. id:%s", id.c_str());
		ckey = "NULL";
		return false;
	}
	ckey = row[0];
	mysql_free_result(res);
	return true;
}

/*该函数没有用了*/
bool mydb_operations::set_wurl_ckey(const string &wurl,
									const string &ckey, 
									const string &tb_name,
									MYSQL *con)
{
	string records;
/* TODO 不明白 */
	bool flag = 
		mydb_operations::get_record_from_ckey(ckey, tb_name, records, con);
	if (!flag) 
		return false;

	stringstream ss(records);
	string ret, url, host, size;
	if (getline(ss, url, '^') && 
			getline(ss, host, '^') && 
			getline(ss, size, '^')) {
		if (!mydb_operations::
				set_ckey_curl(ckey, url, host, size, 
								wurl, tb_name, ret, con)) {
			LOG_WARN("SET RECORD FAILED MESSAGE: %s", ret.c_str());
			return false;
		}
		return true;
	}
	LOG_WARN("bad return format %s", records.c_str());
	return false;
}

/*该函数没有用了*/
bool mydb_operations::get_record_from_ckey(const std::string &ckey, 
										const string &tb_name, 
										string &ret,
										MYSQL *con) {
#if 0
	ret.clear();
	string sql = "SELECT ckey, curl, host, size, wurl FROM `" 
					+ tb_name + "` WHERE ckey = '" + ckey + "';";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con))
		return false ;

	MYSQL_RES *res = mysql_use_result(con);
	MYSQL_ROW row;
	while((row = mysql_fetch_row(res)) != NULL) {
		ret += row[1];
		ret += "^";
		ret += row[2];
		ret += "^";
		ret += row[3];
		ret += "^";
		ret += row[4];
		ret += "|";
	}
	mysql_free_result(res);
	if(ret.size() == 0){
		ret = "NULL";
		return false;
	}
	return true;
#else
	ret.clear();
	string sql = "SELECT host, curl FROM `" 
					+ tb_name + "` WHERE ckey = '" + ckey + "';";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
		ret = "FAIL";
		return false ;
	}

	MYSQL_RES *res = mysql_use_result(con);
	if (res == NULL) {
		ret = "FAIL";
		return false ;
	}
	MYSQL_ROW row;
	int first = 0;
	while((row = mysql_fetch_row(res)) != NULL) {
		if (first != 0) {
			ret += "^";
		} else {
			first = 1;
		}
		ret += row[0];
		ret += "^";
		ret += row[1];
	}
	mysql_free_result(res);
	if(ret.size() == 0){
		ret = "NULL";
		return false;
	}
	return true;
#endif
}

bool mydb_operations::set_ckey_curl(
		const string &ckey, const string &curl, 
		const string &host, const string &size, 
		const string &wurl, const string &tb_name, 
		string &ret, MYSQL *con)
{
	bool retcode;	
	string tempckey;
	get_ckey_from_wurl(wurl, tb_name, tempckey, con);
	if (tempckey != "NULL") {
		if (tempckey == "FAIL") {
			ret = "exec insert sql failed.";
		} else {
			ret = "This wurl record has exist in the database";
		}
		return false;
	}

	string sql;
	sql += "INSERT INTO `" + tb_name 
		+ "` (wurl, ckey, curl, host, size, error) VALUES ('";

	string wurl_new = adjust_wurl_for_sql(wurl);
	//TODO for test  0514
	//check_youku_wurl(wurl_new);

	sql += wurl_new + "', '";

	sql += ckey + "', '";
	sql += curl + "', '";
	sql += host + "', ";
	sql += size + ", 0);";
	//cout << sql << endl;
	retcode = mydb_operations::exec_sql(sql.c_str(), ret, con);	
	if(!retcode) {
		ret = "exec insert sql failed.";
		return false;
	} else {
		ret = "Successfully insert data into the database";
		return true;		
	}
}

/*已废弃*/
bool mydb_operations::set_ckey_curl_have_id(
		const string &ckey, const string &curl, 
		const string &host, const string &size, 
		const string &wurl, const string &id,
		const string &tb_name, string &ret, 
		MYSQL *con)
{
	bool retcode;	
	string tempckey = "NULL";
	get_ckey_from_wurl(wurl, tb_name, tempckey, con);
	if( tempckey != "NULL" ) {
		ret = "This wurl record has exist in the database";
		return false;
	}

	string sql;
#if 0 //for test 2014/04/29
	sql += "INSERT INTO `" + tb_name 
		+ "` (wurl, ckey, curl, host, size, error, id) VALUES ('";
	string wurl_new = adjust_wurl_for_sql(wurl);
	sql += wurl_new + "', '";
	//sql += wurl + "', '";
	sql += ckey + "', '";
	sql += curl + "', '";
	sql += host + "', ";
	sql += size + ",";
	sql += "0, '";
	sql += id + "');";
#else
	sql += "INSERT INTO `" + tb_name 
		+ "` (wurl, ckey, curl, host, size, error) VALUES ('";
	string wurl_new = adjust_wurl_for_sql(wurl);
	sql += wurl_new + "', '";
	//sql += wurl + "', '";
	sql += ckey + "', '";
	sql += curl + "', '";
	sql += host + "', ";
	sql += size + ",";
	sql += "0);";
#endif
	//cout << sql << endl;
	retcode = mydb_operations::exec_sql(sql.c_str(), ret, con);
	if(!retcode) {
		ret = "exec insert sql failed .";
		return false;
	} else {
		ret = "Successfully insert data into the database";
		return true;
	}
}

bool mydb_operations::del_curl(const string &curl,
								const string &tb_name,
								vector <string> &wurls,
								string &ret,
								MYSQL *con)
{
	string sql = "SELECT  wurl FROM `" 
				+ tb_name + "` WHERE curl = '" + curl + "';";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
		ret = "Deleted Failed :" + string(mysql_error(con));
		return false;
	}

	MYSQL_RES *res = mysql_use_result(con);
	MYSQL_ROW row;
	
	string wurl;
	wurls.clear();
	if (res == NULL) {
		ret = "Deleted Failed :" + string(mysql_error(con));
		return false;
	}

	while ((row = mysql_fetch_row(res)) != NULL) {
		wurl = row[0];
		wurls.push_back(wurl);
	}

	if (wurls.empty()) {
		ret = "To delete data in the database does not exist, delete failed.";
		mysql_free_result(res);
		return false;
	} else {
		mysql_free_result(res);
		string del_sql = "DELETE FROM `" + tb_name + "` WHERE curl = '" + curl + "';";
		bool retcode = mydb_operations::exec_sql(del_sql.c_str(), ret, con);
		if (!retcode) {
			ret = "Deleted Failed :" + string(mysql_error(con));
			return false ;
		} else {
			ret = "Deleted successfully.";
			return true;
		}
	}
	return true ;
}



bool mydb_operations::delay_del(const string &host,
                                const string &tb_name,
								vector <string> &wurls,
								string &ret,
								MYSQL *con)
{
    //查询ckey
    //SELECT DISTINCT ckey FROM cc_cachedb.delay_del_wurl WHERE host = '0.0.0.0';
    string sql = "SELECT DISTINCT ckey FROM cc_cachedb.delay_del_wurl WHERE host = '" + host + "';";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
		ret = "Delay Delete Failed :" + string(mysql_error(con));
		return false;
	}

	MYSQL_ROW row;
	MYSQL_RES *res = mysql_use_result(con);
	if (res == NULL) {
		ret = "Delay Deleted Failed :" + string(mysql_error(con));
		return false;
	}

    string ckey;
    while ((row = mysql_fetch_row(res)) != NULL) {
		ckey = row[0];
        //删除数据 DELETE FROM cc_cachedb.`data` WHERE ckey = '1dd9f98d50ca19f6edd12ee4ecb6f862';
        sql = "DELETE FROM cc_cachedb.`" + tb_name + "` WHERE ckey = '" + ckey + "';";
        if (!mydb_operations::exec_sql(sql.c_str(), ret, NULL)) {
        //if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
            ret = "Delay Delete Failed :" + string(mysql_error(con));
            return false;
        }
	}
	mysql_free_result(res);

    //查询已删除的wurl
    sql = "SELECT wurl FROM cc_cachedb.delay_del_wurl WHERE host = '" + host + "';";
    if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
        ret = "Delay Delete Failed :" + string(mysql_error(con));
        return false;
    }
	res = mysql_use_result(con);
	if (res == NULL) {
		ret = "Delay Deleted Failed :" + string(mysql_error(con));
		return false;
	}
    while ((row = mysql_fetch_row(res)) != NULL) {
		wurls.push_back(row[0]);
	}
	mysql_free_result(res);

    //删除delay_del_wurl表
    sql = "DELETE FROM cc_cachedb.delay_del_wurl WHERE host = '" + host + "';";
    if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
        ret = "Delay Delete Failed :" + string(mysql_error(con));
        return false;
    }

    ret = "Ok.";
	return true;
}

bool mydb_operations::delay_del_report(const string &curl,
                                const string &host,
                                const string &tb_name,
								string &ret,
								MYSQL *con)
{
    //FIXME 要写成存储过程

    //查询curl对应的ckey
    //select ckey from cc_cachedb.data where curl = 'curl' limit 1;
    string sql = "select ckey from cc_cachedb." + tb_name + " where curl = '" + curl + "' limit 1;";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
		ret = "Delay Delete Report Failed :" + string(mysql_error(con));
		return false;
	}

	MYSQL_ROW row;
	MYSQL_RES *res = mysql_use_result(con);
	if (res == NULL) {
		ret = "Delay Delete Report Failed :" + string(mysql_error(con));
		return false;
	}

    string ckey;
    if ((row = mysql_fetch_row(res)) != NULL) {
		ckey = row[0];
	} else {
        /*上报删除的数据库中不存在*/
        LOG_WARN("The curl:%s not in db", curl.c_str());
		mysql_free_result(res);
        ret = "Ok.";
        return true;
    }
	mysql_free_result(res);

    //查询延迟删除的wurl(用于写A文件),并将延迟删除的wurl写入到delay_del_wurl表中
    //INSERT INTO cc_cachedb.delay_del_wurl(ckey,wurl) SELECT ckey,wurl FROM cc_cachedb.`data` WHERE ckey = 'ckey'
    sql = "INSERT INTO delay_del_wurl(ckey, wurl) SELECT ckey, wurl FROM " + tb_name + " WHERE ckey = '" + ckey + "';";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
		ret = "Delay Delete Report Failed :" + string(mysql_error(con));
		return false;
	}
    //UPDATE cc_cachedb.delay_del_wurl SET `host` = '0.0.0.0' WHERE ckey = '4ebceba4a7e6769af9e6f4ae5b59c150';
    sql = "UPDATE delay_del_wurl SET `host` = '" + host + "' WHERE ckey = '" + ckey + "';";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
		ret = "Delay Delete Report Failed :" + string(mysql_error(con));
		return false;
	}
    ret = "Ok.";
	return true ;
}

bool mydb_operations::get_resource_size_by_curl(const string &curl,
								const string &tb_name,
								string &ret,
								MYSQL *con)
{
	string sql = "SELECT  size FROM `"
				+ tb_name + "` WHERE curl = '" + curl + "' LIMIT 1" + ";";
	if (!mydb_operations::exec_sql(sql.c_str(), ret, con)) {
		ret = "Get FileSize Failed. " + string(mysql_error(con));
		return false;
	}

	MYSQL_RES *res = mysql_use_result(con);
	if (res == NULL) {
		ret = "Get FileSize Failed, The url no exist." + string(mysql_error(con));
		return false;
	}

	MYSQL_ROW row;
    if ((row = mysql_fetch_row(res)) != NULL) {
		ret = row[0];
	} else {
		ret = "Get FileSize Failed, DB exception." + string(mysql_error(con));
        mysql_free_result(res);
		return false;
    }

    mysql_free_result(res);
	return true ;
}
