/**
 * author: Soli
 * date  : 2013-07-09
 * */
#ifndef __SLIB_PATH_H__
#define __SLIB_PATH_H__

#include <string>

namespace slib
{

std::string get_app_path();
std::string realpath(const char *path);
std::string realpath(const std::string &path);
std::string pathname(const std::string& path_name);
std::string basename(const std::string& path_name);
int mkdir(const std::string& path);
int rmdir(const std::string& path);
int rm(const std::string& path);

} // namespace slib

#endif	// #ifndef __SLIB_PATH_H__

