#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openssl/md5.h>

#include "my_log.h"
#include "my_md5.h"

using namespace std;
/* TODO 和老的计算data长度不一样,一定不能直接搬到download或delete模块 */
#define MD5_DATA_LEN (1 << 20)

static bool get_filesize(const string &filename, unsigned long long filesize)
{
	struct stat statbuff;
	if (stat(filename.c_str(), &statbuff) < 0){
		LOG_ERRO("stat fail:%s %s", filename.c_str(), strerror(errno));
		return false;
	}
	filesize = statbuff.st_size;
	return true;
}

/* 取文件头,中间,末尾三部分数据计算MD5值 */
bool do_md5(ifstream &is, unsigned long long size, string &md5) 
{
	MD5_CTX ctx;
	MD5_Init(&ctx);

	char tmp[MD5_DATA_LEN] = {0};
	is.read(tmp, sizeof(tmp));
	unsigned long ret = is.gcount();
	MD5_Update(&ctx, tmp, ret);

	long m = (size - 1) / 2;
	if (m < 0) m = 0;
	is.seekg(m, ios_base::beg);
	is.read(tmp, sizeof(tmp));
	ret = is.gcount();
	MD5_Update(&ctx, tmp, ret);

	m = size - MD5_DATA_LEN;
	if (m < 0) m = 0;
	is.seekg(m,ios_base::beg);
	is.read(tmp, sizeof(tmp));
	ret = is.gcount();
	MD5_Update(&ctx, tmp, ret);

	unsigned char _md5[MD5_DIGEST_LENGTH] = {0};
	MD5_Final(_md5, &ctx);

	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		unsigned char c = _md5[i];
		int k1 = c >> 4;
		int k2 = c & 0xF;
		md5 += k1 >= 10 ? 'a' + (k1-10) : k1 - 0 + '0';
		md5 += k2 >= 10 ? 'a' + (k2-10) : k2 - 0 + '0';
	}
	return true;
}

bool calc_md5(const string &filename, string &md5) {
	if (filename.empty()) {
		return false;
	}

	ifstream is(filename.c_str(), ios_base::binary | ios_base::in);
	if (!is.is_open()){
		LOG_ERRO("CKEY : get file ckey error %s %s.", 
			filename.c_str(), strerror(errno));
		return false;
	}

	unsigned long long size = 0;
	if (!get_filesize(filename, size)) {
		goto END;
	}

	if (!do_md5(is, size, md5)) {
		goto END;
	}

	is.close();
	return true;
END:
	is.close();
	return false;
}

bool check_md5(const string &filename, const string &md5)
{
	string temp_md5;
	if (!calc_md5(filename, temp_md5)) {
		return false;
	}
	return (temp_md5 == md5);
}

