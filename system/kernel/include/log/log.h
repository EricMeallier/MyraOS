#ifndef LOG_H
#define LOG_H

#include <print/print.h>

#define klog_info(msg) kprintf("INFO: %s\n", msg)
#define klog_warn(msg) kprintf("WARN: %s\n", msg)
#define klog_error(msg) kprintf("ERROR: %s\n", msg)

#endif // LOG_H