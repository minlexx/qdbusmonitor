#include "dbusmonitorthread.h"
#include "dbusmonitorthread_p.h"


DBusMonitorThread::DBusMonitorThread(QObject *parent)
    : QThread(parent)
    , d_ptr(new DBusMonitorThreadPrivate(this))
{
}


bool DBusMonitorThread::startOnSessionBus()
{
    Q_D(DBusMonitorThread);
    return d->startBus(DBUS_BUS_SESSION);
}

bool DBusMonitorThread::startOnSystemBus()
{
    Q_D(DBusMonitorThread);
    return d->startBus(DBUS_BUS_SYSTEM);
}

bool DBusMonitorThread::isMonitorActive() const
{
    Q_D(const DBusMonitorThread);
    return d->m_monitor_active;
}

void DBusMonitorThread::run()
{
    Q_D(DBusMonitorThread);
    d->run();
}
