#include<stdlib.h>
#include<iostream>
using namespace std;

class data_location
{
private:
	int	m_machine_base_id;
	int	m_machine_id;
	int	m_slave_machine_id;
	int	m_database_id;
	int	m_table_id;
	int	m_sum;

public:
        /**
      	* 数据存放区域对象构造函数.
        * @param String url_md5_str      要判断存放区域的URL的32位的MD5编码
        * @param int max_machine_num     数据存放机器数的上限值
     	* @param int current_machine_num 当前用于数据存放机器数
     	* @param int db_database_num     每台机器存放数据的数据库数量
     	* @param int db_table_num        每台机器存放数据的每个数据库中所能使用的数据表的数量
       	*/
	data_location(string url_md5_str,int max_machine_num=32,int current_machine_num=4,int db_database_num=100,
	int db_table_num=100):m_machine_base_id(0),m_machine_id(0),m_slave_machine_id(0),
	m_database_id(0),m_table_id(0),m_sum(0)
	{
		get_machine_id(url_md5_str,max_machine_num,current_machine_num);
        	get_slave_machine_id(current_machine_num);
        	get_database_id(url_md5_str,db_database_num);
        	get_table_id(url_md5_str,db_table_num);	
	}
	

	/**
     	* 得到数据存放的数据表名称.
     	* @param String 数据存放的数据表名称
     	*/
	void 	get_table_name(string &out)
	{
		char buf[6] ;    //= (char *)malloc(6);
        	sprintf(buf,"tb_%03d",m_table_id);
		out.assign(buf);
		//return table_name;
	}

	/**
     	* 得到数据存放的数据库名称.
     	* @param String 数据存放的数据库名称
     	*/
	void  get_database_name( string & out)
	{
		char temp[9] ;
		sprintf(temp,"db_%02d_%03d",m_machine_base_id,m_database_id);
		out.assign(temp);
	}

	
	/**
     	* 得到数据存放的机器_id.
     	* @return int 数据存放的机器_id
     	*/
    	int	get_machine_num() const
    	{
        	return m_machine_id;
    	}

    	/**
     	* 得到数据存放的备份机器_id.
     	* @return int 数据存放的备份机器_id
     	*/
    	int	get_slave_machine_num() const 
    	{
        	return m_slave_machine_id;
    	}

private:
	void 	get_machine_id(string str,int max_num,int current_num)
	{
		m_machine_base_id = get_convert_id(str,max_num,1,10);
        	m_machine_id = (m_machine_base_id%current_num);
	}
	void	get_slave_machine_id(int current_num)
	{
		m_slave_machine_id = ((m_machine_base_id+1)%current_num);
	}
	void	get_database_id(string md5str,int num)
	{
		m_database_id = get_convert_id(md5str,num,11,21);

	}
	void	get_table_id(string md5str,int num)
	{
		m_table_id = get_convert_id(md5str,num,22,32);
	}

	/**
     	* 根据32位的MD5值计算相应的Hash值.
     	* 依据指定截取MD5值的字符串(给定的要截取MD5值起始位置和终止位置),将字符串中每个字符的asci值求和,
     	* 得到该求和值对指定数据进行取余数,得到最终hash值.
     	* @param String str 要进行hash的32为md5值
     	* @param int max_num 要进行取余的数值
     	* @param int start_pos 要截取的32位的md5字符串的起始位置
     	* @param int end_pos 要截取的32位的md5字符串的终止位置
     	* @return int Hash结果.
     	*/
	int	get_convert_id(string str,int max_num,int start_pos,int end_pos)
	{
		int start=start_pos-1;
		string sub=str.substr(start,end_pos);	
		for(const char *p=sub.c_str() ; (*p)!='\0' ; p++ )
               		m_sum += *p ;
		return (m_sum%max_num);
	}
};

