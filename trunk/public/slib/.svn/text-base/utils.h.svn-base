/**
 * author: Soli
 * date  : 2013-07-07
 * */
#ifndef __SLIB_UTILS_H__
#define __SLIB_UTILS_H__

#include <string>

namespace slib
{

std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);
bool file2str(const std::string &file_name, std::string &content);
bool str2file(const std::string &content, const std::string &file_name);
bool append_str2file(const std::string &content, const std::string &file_name);
int wget(const std::string &url, const std::string &path);
int md5sum(const std::string &path, std::string &md5);
int tarx(const std::string &tar, const std::string &path);
bool bin2hex(const char *bin, size_t len, std::string &hex);
std::string bin2hex(const char *bin, size_t len);
void int2hex(uint32_t i, std::string &hex);
std::string int2hex(uint32_t i);
bool SHA1(const std::string &plain, std::string &secret);

} // namespace slib

#endif	// #ifndef __SLIB_UTILS_H__
