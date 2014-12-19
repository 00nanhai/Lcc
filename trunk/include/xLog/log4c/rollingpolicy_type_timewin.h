#ifndef log4c_policy_type_timewin_h
#define log4c_policy_type_timewin_h

#include <log4c/defs.h>
#include <log4c/rollingpolicy.h>

__LOG4C_BEGIN_DECLS

LOG4C_API const log4c_rollingpolicy_type_t log4c_rollingpolicy_type_timewin;

typedef struct __timewin_udata rollingpolicy_timewin_udata_t;

#define TIME_TYPE_HOUR 0
#define TIME_TYPE_DAY 1
#define TIME_TYPE_MONTH 2
#define TIME_TYPE_YEAR 3
#define TIME_TYPE_MIN 4
#define TIME_TYPE_5MIN 5
#define TIME_TYPE_10MIN 6
#define TIME_TYPE_30MIN 7


#define ROLLINGPOLICY_TIME_DEFAULT_MAX_NUM_FILES 10000000
#define ROLLINGPOLICY_TIME_DEFAULT_FILE_TYPE TIME_TYPE_DAY

LOG4C_API rollingpolicy_timewin_udata_t *timewin_make_udata(void);

LOG4C_API int timewin_udata_set_file_time_type(
			rollingpolicy_timewin_udata_t * swup,
			long time_type);

LOG4C_API int timewin_udata_set_max_num_files(
			rollingpolicy_timewin_udata_t * swup,
			long max_num);

LOG4C_API int timewin_udata_set_appender(
			rollingpolicy_timewin_udata_t * swup,
			log4c_appender_t* app);

__LOG4C_END_DECLS


#endif
