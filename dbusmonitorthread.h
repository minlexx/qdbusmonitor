#ifndef DBUSMONITORTHREAD_H
#define DBUSMONITORTHREAD_H

#include <QObject>
#include <QThread>

#include <dbus/dbus.h>


class DBusMonitorThread: public QThread
{
    Q_OBJECT

public:
    explicit DBusMonitorThread(QObject *parent = nullptr);

    bool startOnSessionBus();
    bool startOnSystemBus();

protected:
    void run() override;
    bool becomeMonitor();
    bool startBus(DBusBusType type = DBUS_BUS_SESSION);
    void closeDbusConn();

    static DBusHandlerResult monitorFunc(
            DBusConnection *connection,
            DBusMessage    *message,
            void           *user_data);

Q_SIGNALS:
    void dbusDisconnected();

private:
    DBusConnection *m_dconn = nullptr;
};

#endif // DBUSMONITORTHREAD_H
