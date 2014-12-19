#include<iostream>
#include<string>
#include<stdlib.h>
#include<map>
#include<vector>
using namespace std;

 //字符串分割函数
vector<string> split(string &src, string pattern)
{
    vector<string> strs;
    int pattern_len = pattern.size();
    int last_position = 0, index = -1;
    while (-1 != (index = src.find(pattern, last_position)))
    {
        strs.push_back(src.substr(last_position,index - last_position));
        last_position = index + pattern_len;
    }
    string last_string = src.substr(last_position);
    //if (!lastString.empty())
    strs.push_back(last_string);
    return strs;
}

