#ifndef DBUSMONITORTHREAD_H
#define DBUSMONITORTHREAD_H

#include <QThread>

#include "libqdbusmonitor.h"
#include "dbusmessageobject.h"


class DBusMonitorThreadPrivate;

class LIBQDBUSMONITOR_API DBusMonitorThread: public QThread
{
    Q_OBJECT
    Q_PROPERTY(bool isMonitorActive READ isMonitorActive NOTIFY isMonitorActiveChanged)

public:
    explicit DBusMonitorThread(QObject *parent = nullptr);

    bool startOnSessionBus();
    bool startOnSystemBus();
    bool isMonitorActive() const;

protected:
    void run() override;

Q_SIGNALS:
    void isMonitorActiveChanged();
    void dbusDisconnected();
    void messageReceived(DBusMessageObject messageObj);

private:
    DBusMonitorThreadPrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(DBusMonitorThread)
};


#endif // DBUSMONITORTHREAD_H
