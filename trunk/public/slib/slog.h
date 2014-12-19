/**
 * author: Soli
 * date  : 2013-08-27
 * */
#ifndef __SLIB_SLOG_H__
#define __SLIB_SLOG_H__

/* add #include here */
#include <stdio.h>
#include <pthread.h>
#include <limits.h>

namespace slib
{

/**
 * log level
 * */
typedef enum
{
	SLOG_LEVEL_NONE	= 0,
	SLOG_LEVEL_DATA	= 1,
	SLOG_LEVEL_ERRO	= 2,
	SLOG_LEVEL_WARN	= 3,
	SLOG_LEVEL_INFO	= 4,
	SLOG_LEVEL_DEBU = 5,
	SLOG_LEVEL_UNKN = 6,
} slog_level;

/**
 * defaults and max
 * */
#define SLOG_LEVEL_DEFAULT            SLOG_LEVEL_INFO
#define SLOG_ROTATE_PERIOD_DEFAULT    3600
#define SLOG_ROTATE_FORMAT_DEFAULT    ".%Y-%m-%d_%H"
#define SLOG_LINE_FORMAT_DEFAULT      "[%Y-%m-%d %H:%M:%S][{LVL}] {MSG}\n"
#define SLOG_LINE_MAX                 1024

/**
 * log struct
 * */
typedef struct slog_s
{
	FILE *fp;
	slog_level lvl;
	pthread_mutex_t lock;
	pthread_t  tid;
	int    running;	// for slog_rotate_thread
	char   full_name[PATH_MAX];  // "./logs/xxx.log"
	char   rotate_fmt[PATH_MAX]; // "./logs/xxx.log.%Y-%m-%d_%H"
	char*  (*rotate_fmt_cb)(char* fmt);
	time_t rotate_time;
	time_t rotate_period;
	void   (*rotate_cb)(const char* rotate_name);
} slog_t;

/**
 * open a log using default params, and return the handle.
 * */
slog_t* slog_open(const char * full_name);

/**
 * set log level
 * */
void slog_setlvl(slog_t * log, slog_level lvl);

/**
 * set log level by level name
 * */
void slog_setlvl_byname(slog_t * log, const char * lvl);

/**
 * set log rotate period (seconds)
 * */
void slog_set_rotate_period(slog_t * log, time_t period);

/**
 * set rotate name format
 * */
void slog_set_rotate_fmt(slog_t * log, const char* fmt);

/**
 * set rotate name format callback
 * */
void slog_set_rotate_fmt_cb(slog_t * log, char*(*fmt_cb)(char* ));

/**
 * set rotate name callback
 * */
void slog_set_rotate_cb(slog_t * log, void (*rotate_cb)(const char* ));

/**
 * write log with customized line format 
 * 
 * line format support all strftime format and bellows:
 * 
 *   + {USEC} - microseconds of current time
 *   + {LVL}  - log level
 *   + {TID}  - thread id
 *   + {MSG}  - message format string
 *
 * example:
 *
 *   "[%Y-%m-%d %H:%M:%S.{USEC}] [{LVL}] [{TID}] {MSG}"
 * */
void slog_write_with_format(slog_t * log, slog_level lvl, const char *line_format, const char *msg_format, ...);

/**
 * close the log.
 * */
void slog_close(slog_t * log);

/**
 * smart macros
 * */
#define slog_write(log, lvl, msg_format, args...)    slog_write_with_format(log, lvl, SLOG_LINE_FORMAT_DEFAULT, msg_format, ##args)
#define SLOG_DATA(log, f, args...)     slog_write(log, SLOG_LEVEL_DATA, f, ##args)
#define SLOG_ERRO(log, f, args...)     slog_write(log, SLOG_LEVEL_ERRO, f, ##args)
#define SLOG_WARN(log, f, args...)     slog_write(log, SLOG_LEVEL_WARN, f, ##args)
#define SLOG_INFO(log, f, args...)     slog_write(log, SLOG_LEVEL_INFO, f, ##args)
#define SLOG_DEBU(log, f, args...)     slog_write(log, SLOG_LEVEL_DEBU, "<%s:%d:%s> "f, __FILE__, __LINE__, __FUNCTION__, ##args)
#define SLOG_UNKN(log, f, args...)     slog_write(log, SLOG_LEVEL_UNKN, "<%s:%d:%s> "f, __FILE__, __LINE__, __FUNCTION__, ##args)

#ifdef SLIB_SLOG_HANDLE
#define LOG_DATA(f, args...)       SLOG_DATA(SLIB_SLOG_HANDLE, f, ##args)
#define LOG_ERRO(f, args...)       SLOG_ERRO(SLIB_SLOG_HANDLE, f, ##args)
#define LOG_WARN(f, args...)       SLOG_WARN(SLIB_SLOG_HANDLE, f, ##args)
#define LOG_INFO(f, args...)       SLOG_INFO(SLIB_SLOG_HANDLE, f, ##args)
#define LOG_DEBU(f, args...)       SLOG_DEBU(SLIB_SLOG_HANDLE, f, ##args)
#define LOG_UNKN(f, args...)       SLOG_UNKN(SLIB_SLOG_HANDLE, f, ##args)
#endif


} // namespace slib

#endif	// #ifndef __SLIB_SLOG_H__
