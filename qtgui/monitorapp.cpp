#include <QQmlContext>
#include <QDebug>
#include <QLoggingCategory>

#include "monitorapp.h"


Q_LOGGING_CATEGORY(logApp, "monitor.app")


MonitorApp::MonitorApp(int &argc, char **argv)
    : QGuiApplication(argc, argv)
{
}


bool MonitorApp::init()
{
    // context properties first
    m_engine.rootContext()->setContextProperty(QLatin1String("app"), this);
    m_engine.rootContext()->setContextProperty(QLatin1String("monitor"), &m_thread);
    // load main QML
    m_engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (m_engine.rootObjects().isEmpty()) {
        qCWarning(logApp) << "Failed to init QML engine! Some error in QML files.";
        return false;
    }

    QObject::connect(&m_engine, &QQmlEngine::quit, [this] () {
        qCDebug(logApp) << "should_exit!";
        m_should_exit = true;
        Q_EMIT this->shouldExitChanged();
        // Q_EMIT this->aboutToQuit(); // cannot emit private signals
        this->quit();
    });

    qRegisterMetaType<DBusMessageObject>();

    QObject::connect(&m_thread, &DBusMonitorThread::messageReceived,
                     this, &MonitorApp::onMessageReceived);

    return true;
}

QQmlApplicationEngine *MonitorApp::engine() { return &m_engine; }

bool MonitorApp::shouldExit() const { return m_should_exit; }

QObject *MonitorApp::messagesModelObj() { return static_cast<QObject *>(&m_messages); }

void MonitorApp::startOnSessionBus() { m_thread.startOnSessionBus(); }

void MonitorApp::startOnSystemBus() { m_thread.startOnSystemBus(); }

void MonitorApp::stopMonitor()
{
    if (m_thread.isRunning()) {
        m_thread.requestInterruption();
        m_thread.wait(1000);
    }
}

void MonitorApp::clearLog()
{
    m_messages.clear();
}

void MonitorApp::onMessageReceived(const DBusMessageObject &dmsg)
{
    m_messages.addMessage(dmsg);
    Q_EMIT autoScroll();
}
