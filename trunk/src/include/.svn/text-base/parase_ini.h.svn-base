#include<iostream>
#include<string>
#include<stdlib.h>
#include<map>
using namespace std;

#define CONFIG "config.ini"
#define EQU "="
#define SPACE " "
#define CR "\n"
#define CRLF "\r\n"
class parase_ini
{
	public:
		map<string,string> config;
	
 		parase_ini(const char *config_file=NULL)
		{
			file=(config_file==NULL)?CONFIG:config_file;
		}

		map<string,string> get_config()
		{
			config=read_conf((char *)file); //载入配置文件
                        if(config.empty())
                        {
                                cout<<"Then config file is empty."<<endl;
                                return config;
                        }
			return config;
		}

	private:
		const char *file;
		map<string,string> read_conf(char *FileName)
		{
        		FILE *fp;
        		char line[512];
        		char *note=";";   //注释
        		char *s1;
        		char *s2;
        		map<string,string> m_data;
        		if((fp=fopen(FileName,"rb"))==NULL)
        		{
                		printf("------can not open:%s\n",FileName);
                		return m_data;
        		}
        		while(fgets(line,512,fp))
        		{
                		/*判断此行是否为注释行,去掉注释行*/
               	 		if (!strlen(line) || !strncmp(line,note,1) || !strcmp(line,CR) || !strcmp(line,CRLF))
                        	continue;
                		s1=strtok(line,EQU);
                		s2=strtok(NULL,CRLF);
                		if (!s2)
                        		s2 = strtok(NULL,CR);                        /*判断每行有没有回车符*/

                		if(!s1 || !s2)
                        		continue;
                		m_data[s1]=s2;

        		}
			fclose(fp);
        		return m_data;
		}


};

