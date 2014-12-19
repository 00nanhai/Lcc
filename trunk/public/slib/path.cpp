/**
 * author: Soli
 * date  : 2013-07-09
 * */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <limits.h>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>

#include "path.h"

namespace slib
{

using namespace std;

//得到当前应用程序的路径
std::string get_app_path()
{
	char proc_exe[] = "/proc/self/exe";
	char app_path[PATH_MAX];

	memset(app_path, 0, sizeof(app_path));

	if(readlink(proc_exe, app_path, sizeof(app_path)-1) > 0)
	{
		char *ptr=strrchr(app_path,'/');
		if(ptr){
			ptr[1]='\0';
		}
		return std::string(app_path);
	}

	return "./";
}

std::string realpath(const char *path)
{
	char real_path[PATH_MAX];
	if(0 != access(path, F_OK | R_OK))
	{
		return "";
	}

	return ::realpath(path, real_path);
}

std::string realpath(const std::string &path)
{
	return realpath(path.c_str());
}

std::string pathname(const std::string& path_name)
{
	std::string path(path_name.c_str());
	size_t t = path.size();

	for(; t > 0; --t)
	{
		if(path[t-1] == '/' || path[t-1] == '\\')
		{
			break;
		}
	}

	for(; t > 0; --t)
	{
		if(path[t-1] != '/' && path[t-1] != '\\')
		{
			break;
		}
	}

	return path.substr(0, t);
}

std::string basename(const std::string& path_name)
{
	std::string path(path_name.c_str());
	size_t head = 0, tail = 0;

	for(size_t t = path.size(); t > 0; --t)
	{
		if(path[t-1] != '/' && path[t-1] != '\\')
		{
			tail = t;
			break;
		}
	}

	for(size_t h = tail; h > 0; --h)
	{
		if(path[h-1] == '/' || path[h-1] == '\\')
		{
			head = h;
			break;
		}
	}

	return path.substr(head, tail - head);
}

int mkdir(const std::string& path)
{
	std::string dir(path.c_str());
	if(dir.empty())
	{
		return -1;
	}

	// 替换反斜杠
	for(size_t i = 0; i < dir.size(); ++i)
	{
		if(dir[i] == '\\')
		{
			dir[i] = '/';
		}
	}

	//
	size_t pos = (dir[0] == '/' ? 1 : 0);
	size_t slash_pos = dir.find('/', pos);

	for( ; pos != std::string::npos; slash_pos = dir.find('/', pos))
	{
		std::string cur_path(dir.substr(0, slash_pos));

		if(slash_pos != std::string::npos)
		{
			pos = slash_pos + 1;
		}
		else
		{
			pos = std::string::npos;
		}

		struct stat filestat;
		if(0 == stat(cur_path.c_str(), &filestat))
		{
			if(!S_ISDIR(filestat.st_mode))
			{
				return -2;
			}

			continue;
		}

		if(0 != ::mkdir(cur_path.c_str(), 0755))
		{
			return -3;
		}
	}

	return 0;
}

int rmdir(const std::string& path)
{
	DIR *dir = opendir(path.c_str());
	if(NULL == dir)
	{
		return -1;
	}

	int err = 0;
	struct dirent * entry;
	for(entry = readdir(dir); NULL != entry; entry = readdir(dir))
	{
		std::string name(entry->d_name);

		if(name == "." || name == "..")
			continue;

		string sub(path + "/" + name);

		if(entry->d_type == DT_DIR)
		{
			err = rmdir(sub);
			if(0 != err)
			{
				break;
			}
		}
		else
		{
			if(0 != ::unlink(sub.c_str()))
			{
				err = -2;
				break;
			}
		}
	}

	closedir(dir);

	if(0 != ::rmdir(path.c_str()))
	{
		return -3;
	}

	return 0;
}

int rm(const std::string& path)
{
	struct stat filestat;
	if(stat(path.c_str(), &filestat)<0)
	{	//文件不存在
		return -1;
	}

	if(S_ISDIR(filestat.st_mode))
	{
		return rmdir(path);
	}

	if(0 != ::unlink(path.c_str()))
	{
		return -2;
	}

	return 0;
}

} // namespace slib

