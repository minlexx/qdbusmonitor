#ifndef DBUSMONITORTHREAD_P_H
#define DBUSMONITORTHREAD_P_H

#include <dbus/dbus.h>
#include <QString>
#include <QHash>
#include <QList>

class DBusMonitorThread;


class DBusMonitorThreadPrivate {
public:
    explicit DBusMonitorThreadPrivate(DBusMonitorThread *parent);
    bool becomeMonitor();
    bool startBus(DBusBusType type = DBUS_BUS_SESSION);
    void closeDbusConn();

    uint queryBusNameUnixPid(const QString &busName);
    QString queryNameOwner(const QString &busName);

    void addNameOwner(const QString &busName, const QString &busAddr);
    void removeNameOwner(const QString &busAddr, const QString &busName);
    void addNamePid(const QString &busName, uint pid);
    QStringList resolveDBusAddressToName(const QString &addr);
    QString resolveNameAddress(const QString &name);
    uint resolvePid(const QString &addr);

    static DBusHandlerResult monitorFunc(
            DBusConnection *connection,
            DBusMessage    *message,
            void           *user_data);

    void run();

public:
    DBusConnection *m_dconn = nullptr;
    DBusConnection *m_dconn2 = nullptr;
    DBusMonitorThread *owner = nullptr;
    QString m_myName;
    QString m_myName2;
    QHash<QString, QStringList> m_addrNames;
    QHash<QString, uint> m_addrPids;
    bool m_monitor_active = false;
};
#endif // DBUSMONITORTHREAD_P_H
