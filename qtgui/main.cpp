#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QVariant>
#include <QDebug>
#include <QLoggingCategory>

#include "dbusmessageobject.h"
#include "dbusmessagesmodel.h"
#include "dbusmonitorthread.h"


Q_LOGGING_CATEGORY(logApp, "monitor.app")


class MonitorApp: public QGuiApplication
{
    Q_OBJECT
    Q_PROPERTY(bool shouldExit READ shouldExit NOTIFY shouldExitChanged)
    Q_PROPERTY(QObject* messagesModel READ messagesModelObj NOTIFY messagesModelChanged)

public:
    MonitorApp(int &argc, char **argv)
        : QGuiApplication(argc, argv) {
        //
    }

    ~MonitorApp() override {
        //
    }

    bool init() {
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

public Q_SLOTS:
    QQmlApplicationEngine *engine() { return &m_engine; }
    bool shouldExit() const { return m_should_exit; }
    QObject *messagesModelObj() { return static_cast<QObject *>(&m_messages); }

    void startOnSessionBus() { m_thread.startOnSessionBus(); }

    void startOnSystemBus() { m_thread.startOnSystemBus(); }

    void stopMonitor() {
        if (m_thread.isRunning()) {
            m_thread.requestInterruption();
            m_thread.wait(1000);
        }
    }

    void clearLog() {
        m_messages.clear();
    }

    void onMessageReceived(const DBusMessageObject &dmsg) {
        m_messages.addMessage(dmsg);
        Q_EMIT autoScroll();
    }

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


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    MonitorApp app(argc, argv);
    if (!app.init()) {
        return -1;
    }

    int mainret = app.exec();
    app.stopMonitor(); // should stop BG thread before exiting app

    return mainret;
}

#include "main.moc"
