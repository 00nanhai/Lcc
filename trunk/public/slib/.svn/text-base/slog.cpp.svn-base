/**
 * author: Soli
 * date  : 2013-08-27
 * */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/time.h>
#include "path.h"
#include "slog.h"


namespace slib
{

static const char *slog_level_names[] = {
	/*[SLOG_LEVEL_NONE]    = */"!!!",
	/*[SLOG_LEVEL_DATA]    = */"DATA",
	/*[SLOG_LEVEL_ERRO]    = */"ERRO",
	/*[SLOG_LEVEL_WARN]    = */"WARN",
	/*[SLOG_LEVEL_INFO]    = */"INFO",
	/*[SLOG_LEVEL_DEBU]    = */"DEBU",
	/*[SLOG_LEVEL_UNKN]    = */"UNKO",
};


static int    slog_do_write(slog_t * log, const char * log_line, size_t log_len);
static void   slog_flush(slog_t * log);
static void*  slog_rotate_thread(void* arg);
static int    slog_rotate(slog_t * log, time_t cur);
static time_t slog_rotate_time(slog_t * log);
static char*  str_replace(const char* str1, const char* str2, const char* str);

/**
 * open a log using default params, and return the handle.
 * */
slog_t* slog_open(const char * full_name)
{
	if(strlen(full_name) + strlen(SLOG_ROTATE_FORMAT_DEFAULT) >= PATH_MAX)
	{
		return NULL;
	}

	slog_t * log = (slog_t*)malloc(sizeof(slog_t));
	if(NULL == log)
	{
		return NULL;
	}

	log->lvl = SLOG_LEVEL_DEFAULT;
	pthread_mutex_init(&(log->lock), NULL);

	log->running       = 1;
	log->rotate_period = SLOG_ROTATE_PERIOD_DEFAULT;
	log->rotate_time   = slog_rotate_time(log);
	log->rotate_fmt_cb = NULL;

	strncpy(log->full_name, full_name, sizeof(log->full_name));
	strncpy(log->rotate_fmt, full_name, sizeof(log->rotate_fmt));
	strcat(log->rotate_fmt, SLOG_ROTATE_FORMAT_DEFAULT);

	if(0 != mkdir(pathname(log->full_name)))
	{
		free(log);
		return NULL;
	}

	log->fp = fopen(log->full_name, "a");
	if(NULL == log->fp)
	{
		free(log);
		return NULL;
	}

	if(0 != pthread_create(&log->tid, NULL, slog_rotate_thread, log))
	{
		fclose(log->fp);
		free(log);
		return NULL;
	}
	return log;
}

/**
 * set log level
 * */
void slog_setlvl(slog_t * log, slog_level lvl)
{
	if(NULL != log)
	{
		log->lvl = lvl;
	}
}

/**
 * set log level by level name
 * */
void slog_setlvl_byname(slog_t * log, const char * lvl)
{
	int l = SLOG_LEVEL_NONE;
	for( ; l < SLOG_LEVEL_UNKN; l++)
	{
		if(0 == strcmp(slog_level_names[l], lvl))
		{
			break;
		}
	}

	if(l == SLOG_LEVEL_UNKN)
		l = SLOG_LEVEL_DEFAULT;

	slog_setlvl(log, (slog_level)l);
}

/**
 * set log rotate period (seconds)
 * */
void slog_set_rotate_period(slog_t * log, time_t period)
{
	if(NULL != log)
	{
		log->rotate_period = period;
		log->rotate_time = slog_rotate_time(log);
	}
}

/**
 * set rotate name format
 * */
void slog_set_rotate_fmt(slog_t * log, const char* fmt)
{
	if(NULL != log)
	{
		strncpy(log->rotate_fmt, fmt, sizeof(log->rotate_fmt));
	}
}

/**
 * set rotate name format callback
 * */
void slog_set_rotate_fmt_cb(slog_t * log, char*(*fmt_cb)(char* ))
{
	if(NULL != log)
	{
		log->rotate_fmt_cb = fmt_cb;
	}
}

/**
 * set rotate name callback
 * */
void slog_set_rotate_cb(slog_t * log, void (*rotate_cb)(const char* ))
{
	if(NULL != log)
	{
		log->rotate_cb = rotate_cb;
	}
}
/**
 * write log with line format
 * */
void slog_write_with_format(slog_t * log, slog_level lvl, const char *line_format, const char *msg_format, ...)
{
	if(lvl > log->lvl)
	{
		return;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	if(log->rotate_time > 0 && tv.tv_sec > log->rotate_time)
	{
		slog_rotate(log, tv.tv_sec);
	}

	if((int)lvl < (int)SLOG_LEVEL_NONE)
	{
		lvl = SLOG_LEVEL_NONE;
	}
	else if((int)lvl > (int)SLOG_LEVEL_UNKN)
	{
		lvl = SLOG_LEVEL_UNKN;
	}

	size_t log_len = 0;
	struct tm tm_s;
	char   line_str[SLOG_LINE_MAX];
	char   time_str[32];

	//line format
	log_len = strftime(line_str, sizeof(line_str), line_format, localtime_r(&(tv.tv_sec), &tm_s));
	line_str[log_len] = '\0';

	log_len = snprintf(time_str, sizeof(time_str), "%06ld", tv.tv_usec);
	time_str[log_len] = '\0';

	//TODO: if(NULL == msg_str)
	char* msg_str = str_replace("{LVL}", slog_level_names[lvl], line_str);
	char* tmp_str = str_replace("{USEC}", time_str, msg_str);
	free(msg_str); msg_str = tmp_str;
	tmp_str = str_replace("{MSG}", msg_format, msg_str);
	free(msg_str); msg_str = tmp_str;

	// log str
	char* log_str = line_str; // 重用现有内存
	log_len = 0;

	if(NULL != log_str)
	{
		va_list args;
		va_start(args, msg_format);
		log_len = vsnprintf(log_str, SLOG_LINE_MAX, msg_str, args);
		va_end(args);
	}

	if(log_len >= SLOG_LINE_MAX)
	{
		char* tmp_p = (char*)malloc(log_len + 1);
	
		if(NULL == tmp_p)
		{
			log_str = NULL;
		}
		else
		{
			log_str = tmp_p;

			va_list args;
			va_start(args, msg_format);
			log_len = vsnprintf(log_str, log_len+1, msg_str, args);
			va_end(args);
		}
	}

	//
	free(msg_str);

	//
	if(NULL != log_str && log_len > 0)
	{
		if( 0 != slog_do_write(log, log_str, log_len))
		{
			perror("fwrite()");
		}
	}

	if(NULL != log_str && log_str != line_str)
	{
		free(log_str);
	}
}

/**
 * close the log.
 * */
void slog_close(slog_t * log)
{
	if(NULL == log)
	{
		return;
	}

	// 
	log->running = 0;

	struct timespec to;
	to.tv_sec = log->rotate_period > 0 ? log->rotate_period : 60;
	to.tv_nsec= 0;
	pthread_timedjoin_np(log->tid, NULL, &to);

	//
	pthread_mutex_lock(&(log->lock));

	if(NULL != log->fp)
	{
		slog_flush(log);
		fclose(log->fp);
		log->fp = NULL;
	}

	pthread_mutex_unlock(&(log->lock));

	free(log);
}

static void* slog_rotate_thread(void* arg)
{
	slog_t* log = (slog_t*)arg;
	if(NULL == log)
	{
		return NULL;
	}

	while(log->running)
	{
		time_t cur = time(NULL);
		if(log->rotate_time > 0 && cur > log->rotate_time)
		{
			slog_rotate(log, cur);
		}

		if(log->rotate_period <= 0 || log->rotate_period > 60)
		{
			sleep(60);
		}
		else
		{
			sleep(log->rotate_period);
		}
	}

	return NULL;
}

/**
 * rotate log file
 * */
static int slog_rotate(slog_t * log, time_t cur)
{
	pthread_mutex_lock(&(log->lock));
	
	if(cur <= log->rotate_time)
	{
		pthread_mutex_unlock(&(log->lock));
		return 0;
	}

	log->rotate_time = slog_rotate_time(log);

	char* fmt = log->rotate_fmt;

	char rotate_fmt[PATH_MAX];
	if(NULL != log->rotate_fmt_cb)
	{
		log->rotate_fmt_cb(rotate_fmt);
		fmt = rotate_fmt;
	}

	// log rotate name
	cur -= log->rotate_period; // adjust time
	char fname[PATH_MAX];
	struct tm tm_s;
	size_t len = strftime(fname, sizeof(fname), fmt, localtime_r(&cur, &tm_s));
	fname[len] = '\0';

	if(0 != mkdir(pathname(fname)))
	{
		pthread_mutex_unlock(&(log->lock));
		return 1;
	}

	fclose(log->fp);

	if(0 == rename(log->full_name, fname))
	{
		if(NULL != log->rotate_cb)
		{
			log->rotate_cb(fname);
		}
	}
	else
	{
		perror("slog_rotate/rename()");
	}

	log->fp = fopen(log->full_name, "a");

	pthread_mutex_unlock(&(log->lock));
	return 0;
}

/**
 * write log to file
 * */
static int slog_do_write(slog_t * log, const char * log_line, size_t log_len)
{
	int ret = 0;

	pthread_mutex_lock(&(log->lock));

	if(1 != fwrite(log_line, log_len, 1, log->fp))
	{
		ret = 1;
	}

	slog_flush(log);

	pthread_mutex_unlock(&(log->lock));

	return ret;
}

/**
 * flush buf to file
 * */
static void slog_flush(slog_t * log)
{
	fflush(log->fp);
}

/**
 * gen next rotating time
 * */
static time_t slog_rotate_time(slog_t * log)
{
	if(NULL == log)
	{
		return -1;
	}

	time_t period = log->rotate_period;

	if(period <= 0)
	{
		return -1;
	}

	// time diff
	struct tm st;
	time_t ct = time(NULL);
	time_t lt, td;
	gmtime_r(&ct, &st);
	lt = mktime(&st);
	td = lt - ct;

	time_t rt = ((ct - td) / period + 1 ) * period;
	rt += td;

	return rt;
}

/**
 * replace str1 by str1 in str
 * */
static char* str_replace(const char* str1, const char* str2, const char* str)
{
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);
	size_t len  = strlen(str);
	size_t dl   = 0;
	size_t l    = 16;
	
	while(l < len) { l *= 2; }

	char* p   = (char*)str;
	char* op  = p;
	char* dstr = (char*)malloc(l);

	if(NULL == dstr)
	{
		return NULL;
	}

	if(len1 == 0)
	{
		strcpy(dstr, str);
		return dstr;
	}

	for(; op < str + len; op = p + len1)
	{
		p = strstr(op, str1);
		if(NULL == p)
		{
			break;
		}

		size_t nl = p - op + len2 + dl;
		if(l <= nl)
		{
			while(l <= nl){ l *= 2; }

			char* tmp = (char*)realloc(dstr, l);
			if(NULL == tmp)
			{
				free(dstr);
				return NULL;
			}

			dstr = tmp;
		}

		if(p > op)
		{
			memcpy(&dstr[dl], op, p - op);
			dl += p - op;
		}

		memcpy(&dstr[dl], str2, len2);
		dl += len2;
	}

	if(op >= str + len)
	{
		dstr[dl] = '\0';
		return dstr;
	}

	size_t nl = str + len - op + dl;
	if(l <= dl)
	{
		while(l <= nl){ l *= 2; }
		
		char* tmp = (char*)realloc(dstr, l);
		if(NULL == tmp)
		{
			free(dstr);
			return NULL;
		}
		dstr = tmp;
	}

	memcpy(&dstr[dl], op, str + len - op);
	dl += str + len - op;
	dstr[dl] = '\0';
	return dstr;
}

} // namespace slib


