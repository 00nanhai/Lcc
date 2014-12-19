/**
 * author: Soli <soli@cbug.org>
 * date  : 2013-07-07
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>
#include "path.h"
#include "utils.h"

namespace slib
{

// trim from start
std::string &ltrim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
std::string &rtrim(std::string &s) 
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
std::string &trim(std::string &s) 
{
    return ltrim(rtrim(s));
}

bool file2str(const std::string &file_name, std::string &content)
{
	FILE *fp = fopen(file_name.c_str(), "r");

	if(NULL == fp)
	{
		return false;
	}

	content.clear();

	char buf[1024];

	while(!feof(fp))
	{
		size_t len = fread(buf, 1, 1024, fp);

		if(ferror(fp))
		{
			fclose(fp);
			return false;
		}

		content.append(buf, len);
	}

	fclose(fp);
	return true;
}

bool str2file(const std::string &content, const std::string &file_name)
{
	FILE *fp = fopen(file_name.c_str(), "w+");

	if(NULL == fp)
	{
		return false;
	}

	size_t wn = fwrite(content.data(), content.size(), 1, fp);
	fclose(fp);

	if(wn != 1)
	{
		return false;
	}

	return true;
}

bool append_str2file(const std::string &content, const std::string &file_name)
{
    FILE *fp = fopen(file_name.c_str(), "a+");

    if(NULL == fp) 
    {   
        return false;
    }   

    size_t wn = fwrite(content.data(), content.size(), 1, fp);
    fclose(fp);

    if(wn != 1)
    {   
        return false;
    }   

    return true;
}

int wget(const std::string &url, const std::string &path)
{
	std::string cmd;

	cmd = "/usr/bin/wget -q \"";
	cmd += url;
	cmd += "\" -O \"";
	cmd += path;
	cmd += "\"";

	return system(cmd.c_str());
}

int md5sum(const std::string &path, std::string &md5)
{
	std::string cmd;

	cmd = "/usr/bin/md5sum \"";
	cmd += path;
	cmd += "\"";

	FILE* fp = NULL;
	fp = popen(cmd.c_str(), "r");
	if(NULL == fp)
	{
		return -1;
	}

	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	char* line = fgets(buffer, sizeof(buffer)-1, fp);
	int ret = pclose(fp);

	if(0 != ret)
	{
		return ret;
	}

	if(NULL == line)
	{
		return -1;
	}

	md5.assign(line, 32);
	return 0;
}

int tarx(const std::string &tar, const std::string &path)
{
	int ret = mkdir(path);
	if(ret != 0)
	{
		return ret;
	}

	std::string cmd;
	cmd = "/bin/tar zxf \"";
	cmd += tar;
	cmd += "\" -C \"";
	cmd += path;
	cmd += "\"";

	return system(cmd.c_str());
}

bool bin2hex(const char *bin, size_t len, std::string &hex)
{
	if(len < 1) return false;

	hex.clear();

	for(size_t i = 0; i < len; ++i)
	{
		char ch = bin[i];

		// 高4位
		ch >>= 4;
		ch &= 0x0f;
		ch += (ch <= 0x09) ? 0x30 : 0x57;

		hex.append(1, ch);

		// 低4位
		ch = bin[i];
		ch &= 0x0f;
		ch += (ch <= 0x09) ? 0x30 : 0x57;

		hex.append(1, ch);
	}
	return true;
}

std::string bin2hex(const char *bin, size_t len)
{
	 std::string hex;
	 if(bin2hex(bin, len, hex))
		 return hex;
	 return "";
}

void int2hex(uint32_t i, std::string &hex)
{
	char buf[32];
	snprintf(buf, 32, "%X", i);
	hex = buf;
}

std::string int2hex(uint32_t i)
{
	char buf[32];
	snprintf(buf, 32, "%X", i);
	return(std::string(buf));
}

bool SHA1(const std::string &plain, std::string &secret)
{
	unsigned char m[SHA_DIGEST_LENGTH];
	if(NULL == ::SHA1((unsigned char*)plain.data(), plain.size(), m))
	{
		return false;
	}

	bin2hex((char*)m, sizeof(m), secret);
	return true;
}

} // namespace slib


