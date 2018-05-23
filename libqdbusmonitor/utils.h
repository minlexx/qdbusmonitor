#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include "libqdbusmonitor.h"

namespace Utils {

LIBQDBUSMONITOR_API QString dbusMessageTypeToString(int message_type);
LIBQDBUSMONITOR_API bool isNumericAddress(const QString &busName);
[[noreturn]] LIBQDBUSMONITOR_API void fatal_oom(const char *where);
LIBQDBUSMONITOR_API QString pid2filename(uint pid);

}

#endif // UTILS_H
