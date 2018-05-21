#ifndef DBUSMONITORTHREAD_H
#define DBUSMONITORTHREAD_H

#include <QObject>
#include <QThread>

#include "dbusmessageobject.h"


class DBusMonitorThreadPrivate;

class DBusMonitorThread: public QThread
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


QString dbusMessageTypeToString(int message_type);


#endif // DBUSMONITORTHREAD_H
