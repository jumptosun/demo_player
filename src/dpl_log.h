#ifndef _DPL_LOG_H_
#define _DPL_LOG_H_

namespace dpl {

enum DplLogLevel {
    DplLogLevelDebug,
    DplLogLevelInfo,
    DplLogLevelTrace,
    DplLogLevelWarn,
    DplLogLevelError,
};

extern void DplLog(DplLogLevel level, const char* file_name, const char* func_name, int line , char* msg, ...);

}

#define dpl_debug(msg, ...)  DplLog(dpl::DplLogLevelDebug, __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__)
#define dpl_info(msg, ...)   DplLog(dpl::DplLogLevelInfo,  __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__)
#define dpl_trace(msg, ...)  DplLog(dpl::DplLogLevelTrace, __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__)
#define dpl_warn(msg, ...)   DplLog(dpl::DplLogLevelWarn,  __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__)
#define dpl_error(msg, ...)  DplLog(dpl::DplLogLevelError, __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__)


#endif
