/*
 * author: Soli
 * date  : 2012-03-16
 */
#include <stdio.h>
#include <fstream>
#include "utils.h"
#include "config.h"

namespace slib
{

Config::Config()
{}

Config::~Config()
{}


bool Config::read(const std::string &file_name)
{
	try
	{
		return _doRead(file_name);
	}
	catch(...)
	{
		perror(("read [" + file_name + "] ").c_str());
		m_kvs.clear();
	}
	return false;
}

bool Config::_doRead(const std::string &file_name)
{
	std::ifstream ifs;
	ifs.open (file_name.c_str());

	std::string line;

	while(ifs.good())
	{
		//读取一行
		std::getline (ifs, line);

		// 去掉注释
		for(size_t sharp = line.find(SLIB_CONFIG_COMMENT_CHAR, 0); 
				sharp != std::string::npos;
				sharp = line.find(SLIB_CONFIG_COMMENT_CHAR, sharp)
		   )
		{
			if(0 == sharp)
			{
				line.clear();
				break;
			}
			else if(line[sharp-1] != '\\')
			{
				line = line.substr(0, sharp);
				break;
			}
			else
			{
				line.erase(sharp-1, 1);
			}
		}

		// 以第一个“=” 分割key和value
		size_t eq = line.find('=');
		if(eq == std::string::npos)
		{
			continue;
		}

		std::string key = line.substr(0, eq);
		std::string value = line.substr(eq+1);

		// 砍掉头尾的空白
		//trim(key);
		//trim(value);
		
		// 砍掉key头尾的双引号
		if(key.size() > 1 && key[0] == '\"' && key[key.size()-1] == '\"')
		{
			key = key.substr(1, key.size()-2);
		}

		// 砍掉value头尾的双引号
		if(value.size() > 1 && value[0] == '\"' && value[value.size()-1] == '\"')
		{
			value = value.substr(1, value.size()-2);
		}

		// 如果key不为空，则存入映射表
		if(!key.empty())
		{
			m_kvs[key] = value;
		}
	}

	ifs.close();
	return true;
}

std::string Config::get(const std::string &key)
{
	if(m_kvs.find(key) != m_kvs.end())
		return m_kvs[key];
	return "";
}

std::string Config::get(const std::string &key, const std::string &def)
{
	if(m_kvs.find(key) != m_kvs.end())
		return m_kvs[key];
	return def;
}

std::string Config::dump()
{
	std::string out;
	std::map<std::string, std::string>::iterator iter = m_kvs.begin();
	for( ; iter != m_kvs.end(); ++iter)
	{
		out.append("\"" + iter->first + "\" = \"" + iter->second + "\"\n");
	}
	return out;
}

}	// namespace slib
