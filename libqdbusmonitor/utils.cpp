#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <dbus/dbus.h>
#include "utils.h"

namespace Utils {


QString dbusMessageTypeToString(int message_type)
{
    switch (message_type)
    {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        return QStringLiteral("method call");
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        return QStringLiteral("method return");
    case DBUS_MESSAGE_TYPE_ERROR:
        return QStringLiteral("error");
    case DBUS_MESSAGE_TYPE_SIGNAL:
        return QStringLiteral("signal");
    default:
        return QStringLiteral("(unknown message type)");
    }
}


bool isNumericAddress(const QString &busName) {
    return busName.startsWith(QLatin1Char(':'));
}


[[noreturn]] void fatal_oom(const char *where)
{
    fprintf(stderr, "Out of memoory: %s", where);
    ::exit(100);
}


#ifdef Q_OS_LINUX
#include <unistd.h>
QString pid2filename(uint pid)
{
    char path[512] = {0};
    char outbuf[512] = {0};
    snprintf(path, sizeof(path) - 1, "/proc/%u/exe", pid);
    readlink(path, outbuf, sizeof(outbuf) - 1);
    return QString::fromUtf8(outbuf);
}
#elif Q_OS_WIN
#include <windows.h>
QString pid2filename(uint pid)
{
    // TODO: did not compile this, verify ;)
    HANDLE hProcess = OpenProcess(static_cast<DWORD>(pid), PROCESS_QUERY_INFORMATION);
    if (hProcess) {
        WCHAR buf[MAX_PATH] = {0};
        GetModuleFileNameW(hProcess, buf, sizeof(buf)/sizeof(buf[0]));
        CloseHandle(hProcess);
        return QString::fromUtf16(buf);
    }
    return QString();
}
#else
QString pid2filename(uint pid)
{
    qDebug() << "pid2filename() not implemented for this platform!";
    return QString();
}
#endif


} // namespace Utils
