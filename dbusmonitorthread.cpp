#include <QDebug>
#include <QLoggingCategory>

#include "dbusmonitorthread.h"

Q_LOGGING_CATEGORY(logMon, "monitor.thread")


#ifdef __cplusplus
#undef DBUS_ERROR_INIT
// fix "Zero as null pointer constant" (change NULL to nullptr)
#define DBUS_ERROR_INIT { nullptr, nullptr, TRUE, 0, 0, 0, 0, nullptr }
#endif

#ifndef DBUS_INTERFACE_MONITORING
// Old libdbus version?
#define DBUS_INTERFACE_MONITORING     "org.freedesktop.DBus.Monitoring"
#endif


[[noreturn]] static void tool_oom(const char *where)
{
    qCCritical(logMon) << "Out of memoory:" << where;
    ::exit(100);
}


DBusMonitorThread::DBusMonitorThread(QObject *parent)
    : QThread(parent)
{
    //
}


static QString dbusMessageTypeToString(int message_type)
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


DBusHandlerResult DBusMonitorThread::monitorFunc(
        DBusConnection     *connection,
        DBusMessage        *message,
        void               *user_data)
{
    Q_UNUSED(connection)
    DBusMonitorThread *thiz = static_cast<DBusMonitorThread *>(user_data);
    qCDebug(logMon) << "DBus message received by filter!";

    if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        // exit(0);
        // QCoreApplication::quit();
        Q_EMIT thiz->dbusDisconnected();
    }

    // get base message properties
    DBusMessageObject messageObj;
    messageObj.sender = QString::fromUtf8(dbus_message_get_sender(message));
    messageObj.destination = QString::fromUtf8(dbus_message_get_destination(message));
    messageObj.type = dbus_message_get_type (message);
    const QString messageType = dbusMessageTypeToString(messageObj.type);

    switch (messageObj.type) {
        case DBUS_MESSAGE_TYPE_METHOD_CALL:
        case DBUS_MESSAGE_TYPE_SIGNAL:
            messageObj.serial = dbus_message_get_serial(message);
            messageObj.path = QString::fromUtf8(dbus_message_get_path(message));
            messageObj.interface = QString::fromUtf8(dbus_message_get_interface(message));
            messageObj.member = QString::fromUtf8(dbus_message_get_member(message));
            break;

        case DBUS_MESSAGE_TYPE_METHOD_RETURN:
            messageObj.serial = dbus_message_get_serial(message);
            messageObj.replySerial = dbus_message_get_reply_serial(message);
            break;

        case DBUS_MESSAGE_TYPE_ERROR:
            messageObj.errorName = QString::fromUtf8(dbus_message_get_error_name(message));
            messageObj.replySerial = dbus_message_get_reply_serial(message);
            break;
    }

    // get message contents
    DBusMessageIter iter;
    dbus_message_iter_init(message, &iter);
    // TODO: get message contents

    // maybe some eother processing required
    Q_EMIT thiz->messageReceived(messageObj);

    // Monitors must not allow libdbus to reply to messages, so we eat the message. See DBus bug 1719.
    return DBUS_HANDLER_RESULT_HANDLED;
}


bool DBusMonitorThread::becomeMonitor()
{
    DBusError error = DBUS_ERROR_INIT;
    DBusMessage *msg = nullptr;
    DBusMessage *replyMsg = nullptr;
    dbus_uint32_t zero = 0;
    DBusMessageIter appender, array_appender;

    msg = dbus_message_new_method_call(DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS,
                                     DBUS_INTERFACE_MONITORING,
                                     "BecomeMonitor");

    if (msg == nullptr) {
        tool_oom ("becoming a monitor");
    }

    dbus_message_iter_init_append(msg, &appender);

    if (!dbus_message_iter_open_container(&appender, DBUS_TYPE_ARRAY, "s", &array_appender)) {
        tool_oom ("opening string array");
    }

    if (!dbus_message_iter_close_container(&appender, &array_appender) ||
            !dbus_message_iter_append_basic(&appender, DBUS_TYPE_UINT32, &zero)) {
        tool_oom ("finishing arguments");
    }

    replyMsg = dbus_connection_send_with_reply_and_block(m_dconn, msg, -1, &error);

    if (replyMsg != nullptr) {
        dbus_message_unref(replyMsg);
    } else if (dbus_error_has_name(&error, DBUS_ERROR_UNKNOWN_INTERFACE)) {
        qCWarning(logMon) << "qdbusmonitor: unable to enable new-style monitoring, "
                             "your dbus-daemon is too old. Falling back to eavesdropping.";
        dbus_error_free(&error);
    } else {
        qCWarning(logMon) << "qdbusmonitor: unable to enable new-style monitoring: "
                          << error.name << ": " << error.message
                          << ". Falling back to eavesdropping.";
        dbus_error_free(&error);
    }

    dbus_message_unref (msg);

    return (replyMsg != nullptr);
}

bool DBusMonitorThread::startOnSessionBus()
{
    return startBus(DBUS_BUS_SESSION);
}

bool DBusMonitorThread::startOnSystemBus()
{
    return startBus(DBUS_BUS_SYSTEM);
}

bool DBusMonitorThread::startBus(DBusBusType type)
{
    m_dconn = nullptr;
    DBusError derror = DBUS_ERROR_INIT;

    dbus_error_init(&derror);
    m_dconn = dbus_bus_get(type, &derror);
    if (!m_dconn) {
        qCWarning(logMon) << "Failed to open dbus connction" << derror.message;
        dbus_error_free(&derror);
        return false;
    }

    // Receive o.fd.Peer messages as normal messages, rather than having
    // libdbus handle them internally, which is the wrong thing for a monitor
    dbus_connection_set_route_peer_messages(m_dconn, TRUE);

    if (!dbus_connection_add_filter(m_dconn, monitorFunc, static_cast<void *>(this), nullptr)) {
        qCWarning(logMon) << "Couldn't add filter!";
        closeDbusConn();
        return false;
    }

    if (!becomeMonitor()) {
        // hack for old dbus server
        dbus_bus_add_match(m_dconn, "eavesdrop=true", &derror);
        if (dbus_error_is_set(&derror)) {
            dbus_error_free(&derror);
            dbus_bus_add_match(m_dconn, "", &derror);
            if (dbus_error_is_set(&derror)) {
                qCWarning(logMon) << "Falling back to eavesdropping failed!";
                qCWarning(logMon) << "Error: " << derror.message;
                closeDbusConn();
                return false;
            }
        }
    }

    start(QThread::LowPriority);
    return true;
}

void DBusMonitorThread::closeDbusConn()
{
    if (m_dconn) {
        dbus_connection_unref(m_dconn);
        m_dconn = nullptr;
    }
}

void DBusMonitorThread::run()
{
    while (dbus_connection_read_write_dispatch(m_dconn, 500)) {
        if (isInterruptionRequested()) {
            qCDebug(logMon) << "Interruption requested, breaking DBus loop";
            break;
        }
    }

    closeDbusConn();
}
