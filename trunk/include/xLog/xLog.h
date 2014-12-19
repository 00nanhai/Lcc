#ifndef _XLOG_H_
#define _XLOG_H_


#ifdef __cplusplus
extern "C"{
#endif


#include <assert.h>
#include <string.h>

#include <log4c.h>
#include <log4c/appender_type_rollingfile.h>
#include <log4c/rollingpolicy.h>
#include <log4c/rollingpolicy_type_sizewin.h>
#include <log4c/rollingpolicy_type_timewin.h>

#define RUN_NONE 0
#define RUN_ERRO 1
#define RUN_WARN 2
#define RUN_INFO 3
#define RUN_DEBU 4

#define xLog_W(format,...) \
	log4c_category_log(log_work, 1, format, ##__VA_ARGS__)

#define xLog_R(priority,area,format,...) \
{ \
	char temp[4096]; \
	snprintf(temp,4096,"%s %s",area,format); \
	log4c_category_log(log_run, priority, temp, ##__VA_ARGS__); \
}

typedef log4c_category_t xLog_category_t;

extern xLog_category_t *log_work;
extern xLog_category_t *log_run;

extern int xLog_init_s();
extern void xLog_createlog_work(xLog_category_t **log_w, const char *logdir_w, const char *prefix);
extern void xLog_createlog_run(xLog_category_t **log_r, const char *logdir_r, const char *prefix, int priority_run);

extern int xLog_init(const char *logdir_r, const char *logdir_w, const char *prefix, int priority_run); 
extern int xLog_fini(void);


#ifdef __cplusplus
}
#endif


#endif

