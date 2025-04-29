#include "dpl_log.h"
#include <cstdio>
#include <cstdarg>
#include <ctime>

namespace dpl {

const char* log_prefix[] = { "DEBUG", "INFO", "TRACE", "WARN", "ERROR" };

void DplLog(DplLogLevel level, const char* file_name, const char* func_name, int line , char* msg, ...)
{
    auto now = time(NULL);
    auto now_l = localtime(&now);

    char now_str[32];
    strftime(now_str, 32, "%Y-%m-%d %H:%M:%S", now_l);

    char log_info[256];
    snprintf(log_info, 256, "[%s][%s: %s: %d][%s]: %s\n", now_str, file_name, func_name, line, log_prefix[level], msg);

    va_list vp;
    va_start(vp, msg);

    if(level > DplLogLevelTrace) {
        vfprintf(stderr, log_info, vp);

    } else {
        vfprintf(stdout, log_info, vp);
    }

    va_end(vp);
}

}
