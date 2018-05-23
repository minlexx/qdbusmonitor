#ifndef MONITORAPP_H
#define MONITORAPP_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "dbusmessagesmodel.h"
#include "dbusmonitorthread.h"


class MonitorApp: public QGuiApplication
{
    Q_OBJECT
    Q_PROPERTY(bool shouldExit READ shouldExit NOTIFY shouldExitChanged)
    Q_PROPERTY(QObject* messagesModel READ messagesModelObj NOTIFY messagesModelChanged)

public:
    MonitorApp(int &argc, char **argv);
    bool init();

public Q_SLOTS:
    QQmlApplicationEngine *engine();
    bool shouldExit() const;
    QObject *messagesModelObj();
    void startOnSessionBus();
    void startOnSystemBus();
    void stopMonitor();
    void clearLog();
    void onMessageReceived(const DBusMessageObject &dmsg);

Q_SIGNALS:
    void shouldExitChanged();
    void messagesModelChanged();
    void autoScroll();

private:
    bool                   m_should_exit = false;
    QQmlApplicationEngine  m_engine;
    DBusMonitorThread      m_thread;
    DBusMessagesModel      m_messages;
};


#endif // MONITORAPP_H
