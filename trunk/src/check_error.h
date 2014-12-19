#ifndef _CHECK_ERROR_
#define _CHECK_ERROR_

#include <assert.h>
#include <string>
#include "my_log.h"

//FIXME add lock
static inline void check_youku_wurl(string wurl) 
{
	string key("youku/");
	if (wurl.compare(0, 6, key, 0, 6) != 0) {
		return;
	}
	//eg valid:youku/030002070550A2BCF2504303BAF2B1D5AC28D2-08C7-4046-A0AD-D036A7EDD26C.flv
	string eg_youku("youku/030002070550A2BCF2504303BAF2B1D5AC28D2-08C7-4046-A0AD-D036A7EDD26C.flv");
	if (eg_youku.length() != wurl.length()) {
		LOG_ERRO("wurl length error. wurl:%s", wurl.c_str());
		//assert(0);
	}
}

#endif
