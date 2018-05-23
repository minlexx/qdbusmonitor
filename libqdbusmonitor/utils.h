#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace Utils {

QString dbusMessageTypeToString(int message_type);
bool isNumericAddress(const QString &busName);
[[noreturn]] void fatal_oom(const char *where);
QString pid2filename(uint pid);

}

#endif // UTILS_H
