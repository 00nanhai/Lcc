/*
 * author: Soli
 * date  : 2012-03-16
 */
#ifndef __SLIB_CONFIG_H__
#define __SLIB_CONFIG_H__

#include <string>
#include <map>

namespace slib
{

#ifndef SLIB_CONFIG_COMMENT_CHAR
#define SLIB_CONFIG_COMMENT_CHAR ';'
#endif

class Config
{
public:
	Config( );
	virtual ~Config();

	bool read(const std::string &file_name);
	std::string get(const std::string &key);
	std::string get(const std::string &key, const std::string &def);
	std::string dump();

private:
	bool _doRead(const std::string &file_name);
	std::map<std::string, std::string> m_kvs;
};


}	// namespace slib

#endif	// #ifndef __SLIB_CONFIG_H__
