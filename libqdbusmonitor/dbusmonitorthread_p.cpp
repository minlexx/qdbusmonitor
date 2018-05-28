#include <QDebug>
#include <QLoggingCategory>
#include "dbusmonitorthread_p.h"
#include "dbusmonitorthread.h"
#include "utils.h"


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


DBusMonitorThreadPrivate::DBusMonitorThreadPrivate(DBusMonitorThread *parent)
    : owner(parent)
{
}


bool DBusMonitorThreadPrivate::becomeMonitor()
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
        Utils::fatal_oom("becoming a monitor");
    }

    dbus_message_iter_init_append(msg, &appender);

    if (!dbus_message_iter_open_container(&appender, DBUS_TYPE_ARRAY, "s", &array_appender)) {
        Utils::fatal_oom("opening string array");
    }

    if (!dbus_message_iter_close_container(&appender, &array_appender) ||
            !dbus_message_iter_append_basic(&appender, DBUS_TYPE_UINT32, &zero)) {
        Utils::fatal_oom("finishing arguments");
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


bool DBusMonitorThreadPrivate::startBus(DBusBusType type)
{
    if (m_dconn || m_dconn2) {
        qCDebug(logMon) << "Already running!";
        return false;
    }
    m_dconn = nullptr;
    m_dconn2 = nullptr;
    const bool DBUSMONITOR_DEBUG = !qgetenv("DBUSMONITOR_DEBUG").isEmpty();

    DBusError derror;
    dbus_error_init(&derror);

    m_dconn = dbus_bus_get(type, &derror);
    if (!m_dconn) {
        qCWarning(logMon) << "Failed to open dbus connction:" << derror.message;
        dbus_error_free(&derror);
        return false;
    }

    // open second private connection to bus
    dbus_error_init(&derror);
    // Unlike dbus_connection_open(), always creates a new connection.
    //   This connection will not be saved or recycled by libdbus.
    m_dconn2 = dbus_bus_get_private(type, &derror);
    if (!m_dconn2) {
        qCWarning(logMon) << "Failed to open second dbus connction:" << derror.message;
        dbus_error_free(&derror);
        closeDbusConn();
        return false;
    }

    m_myName = QString::fromUtf8(dbus_bus_get_unique_name(m_dconn));
    m_myName2 = QString::fromUtf8(dbus_bus_get_unique_name(m_dconn2));
    qCDebug(logMon) << "Connected to D_Bus as: " << m_myName << m_myName2;

    qCDebug(logMon).nospace() << "Compiled with libdbus version: " << DBUS_MAJOR_VERSION
                              << "." << DBUS_MINOR_VERSION
                              << "." << DBUS_MICRO_VERSION;

    int vmajor = 0, vminor = 0, vmicro = 0;
    dbus_get_version(&vmajor, &vminor, &vmicro);
    qCDebug(logMon).nospace() << "Linked libdbus version: " << vmajor
                              << "." << vminor << "." << vmicro;

    // get all known bus names
    DBusMessage *dmsg = dbus_message_new_method_call(
                DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "ListNames");
    if (!dmsg) {
        Utils::fatal_oom("create new message");
    }

    dbus_error_init(&derror);
    DBusMessage *dreply = dbus_connection_send_with_reply_and_block(m_dconn, dmsg, 15000, &derror);
    if (!dreply) {
        qCWarning(logMon) << "Failed to query bus names:" << derror.message;
        dbus_error_free(&derror);
        closeDbusConn();
        return false;
    }

    QStringList knownNames;
    DBusMessageIter diter;
    dbus_message_iter_init(dreply, &diter);
    do {
        int mtype = dbus_message_iter_get_arg_type(&diter);
        if (mtype == DBUS_TYPE_INVALID) {
            break;
        }
        if (mtype == DBUS_TYPE_ARRAY) {
            DBusMessageIter subiter;
            dbus_message_iter_recurse(&diter, &subiter);
            do {
                mtype = dbus_message_iter_get_arg_type(&subiter);
                if (mtype == DBUS_TYPE_INVALID) {
                    break;
                }
                DBusBasicValue value;
                dbus_message_iter_get_basic(&subiter, &value);
                const QString nextName = QString::fromUtf8(value.str);
                knownNames.append(nextName);
            } while(dbus_message_iter_next(&subiter));
        }
    } while(dbus_message_iter_next(&diter));
    dbus_message_unref(dreply);
    dbus_message_unref(dmsg);

    if (DBUSMONITOR_DEBUG) {
        qCDebug(logMon) << " known bus names: " << knownNames;
    }

    // for each known name request its owner and pid
    for (const QString &busName: knownNames) {
        // query name owner
        if (!Utils::isNumericAddress(busName)) {
            const QString nameOwner = queryNameOwner(busName);
            if (!nameOwner.isEmpty()) {
                addNameOwner(busName, nameOwner);
                if (DBUSMONITOR_DEBUG) {
                    qCDebug(logMon) << "  name owner:" << busName << nameOwner;
                }
            }
        }

        // query name PID
        uint namePid = queryBusNameUnixPid(busName);
        if (namePid > 0) {
            addNamePid(busName, namePid);
            if (DBUSMONITOR_DEBUG) {
                qCDebug(logMon) << "  name pid:" << busName << namePid;
            }
        }
    }

    // Receive o.fd.Peer messages as normal messages, rather than having
    // libdbus handle them internally, which is the wrong thing for a monitor
    dbus_connection_set_route_peer_messages(m_dconn, TRUE);

    if (!dbus_connection_add_filter(m_dconn, monitorFunc, static_cast<void *>(owner), nullptr)) {
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

    owner->start(QThread::LowPriority);
    return true;
}


void DBusMonitorThreadPrivate::closeDbusConn()
{
    if (m_dconn) {
        dbus_connection_unref(m_dconn);
        m_dconn = nullptr;
    }
    if (m_dconn2) {
        // For private connections, the creator of the connection
        //  must arrange for dbus_connection_close()
        // When you are done with this connection, you must dbus_connection_close()
        //  to disconnect it, and dbus_connection_unref() to free the connection object.
        dbus_connection_close(m_dconn2);
        dbus_connection_unref(m_dconn2);
        m_dconn2 = nullptr;
    }
    m_monitor_active = false;
}

uint DBusMonitorThreadPrivate::queryBusNameUnixPid(const QString &busName)
{
    uint namePid = 0;
    DBusError derror = DBUS_ERROR_INIT;
    DBusMessage *dmsg = dbus_message_new_method_call(
                DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetConnectionUnixProcessID");
    if (!dmsg) {
        Utils::fatal_oom("create new message");
    }
    std::string stds = busName.toStdString();
    const char *str_ptr = stds.c_str();
    dbus_message_append_args(dmsg, DBUS_TYPE_STRING, &str_ptr, DBUS_TYPE_INVALID);
    dbus_error_init(&derror);
    DBusMessage *dreply = dbus_connection_send_with_reply_and_block(m_dconn2, dmsg, 15000, &derror);
    if (dreply) {
        dbus_error_init(&derror);
        if (!dbus_message_get_args(dreply, &derror, DBUS_TYPE_UINT32, &namePid, DBUS_TYPE_INVALID)) {
            qCWarning(logMon) << "Failed to read name owner pid:" << derror.message;
            dbus_error_free(&derror);
        }
        dbus_message_unref(dreply);
    } else {
        qCWarning(logMon) << "WARNING: Call to GetConnectionUnixProcessID() failed! null reply!"
                          << derror.message;
        dbus_error_free(&derror);
    }
    dbus_message_unref(dmsg);
    return namePid;
}

QString DBusMonitorThreadPrivate::queryNameOwner(const QString &busName)
{
    QString ret;
    DBusError derror = DBUS_ERROR_INIT;
    DBusMessage *dmsg = dbus_message_new_method_call(
                DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetNameOwner");
    if (!dmsg) {
        Utils::fatal_oom("create new message");
    }
    std::string stds = busName.toStdString();
    const char *str_ptr = stds.c_str();
    dbus_message_append_args(dmsg, DBUS_TYPE_STRING, &str_ptr, DBUS_TYPE_INVALID);
    dbus_error_init(&derror);
    DBusMessage *dreply = dbus_connection_send_with_reply_and_block(m_dconn2, dmsg, 15000, &derror);
    if (dreply) {
        dbus_error_init(&derror);
        if (!dbus_message_get_args(dreply, &derror, DBUS_TYPE_STRING, &str_ptr, DBUS_TYPE_INVALID)) {
            qCWarning(logMon) << "Failed to read name owner reply args:" << derror.message;
            dbus_error_free(&derror);
        }
        ret = QString::fromUtf8(str_ptr);
        dbus_message_unref(dreply);
    }
    dbus_message_unref(dmsg);
    return ret;
}


void DBusMonitorThreadPrivate::addNameOwner(const QString &busName, const QString &busAddr)
{
    if (m_addrNames.contains(busAddr)) {
        QStringList &namesList = m_addrNames[busAddr];
        namesList.append(busName);
    } else {
        QStringList namesList{busName};
        m_addrNames[busAddr] = namesList;
    }
}

void DBusMonitorThreadPrivate::removeNameOwner(const QString &busAddr, const QString &busName)
{
    if (m_addrNames.contains(busAddr)) {
        QStringList &namesList = m_addrNames[busAddr];
        namesList.removeAll(busName);
    }
}


void DBusMonitorThreadPrivate::addNamePid(const QString &busName, uint pid)
{
    m_addrPids[busName] = pid;
}


QStringList DBusMonitorThreadPrivate::resolveDBusAddressToName(const QString &addr)
{
    if (addr.isEmpty()) {
        return QStringList();
    }
    if (m_addrNames.contains(addr)) {
        return m_addrNames[addr];
    }
    // qCDebug(logMon) << "Failed to resolve bus addr to name:" << addr;
    // ^^ This is perfectly normal, not every address should have a name on bus
    return QStringList();
}

QString DBusMonitorThreadPrivate::resolveNameAddress(const QString &name)
{
    if (name.isEmpty()) {
        return QString();
    }
    for (const QString &addr: m_addrNames.keys()) {
        const QStringList &names = m_addrNames[addr];
        if (names.contains(name)) {
            return addr;
        }
    }
    qCDebug(logMon) << "Failed to resolve name bus addr:" << name;
    // ^^ This is wrong, every name should have a numeric address
    return QString();
}

uint DBusMonitorThreadPrivate::resolvePid(const QString &addr)
{
    if (addr.isEmpty()) {
        return 0;
    }
    if (m_addrPids.contains(addr)) {
        return m_addrPids[addr];
    }
    qCDebug(logMon) << "Cannot resolve PID for:" << addr;
    return 0;
}


DBusHandlerResult DBusMonitorThreadPrivate::monitorFunc(
        DBusConnection     *connection,
        DBusMessage        *message,
        void               *user_data)
{
    Q_UNUSED(connection)
    DBusMonitorThread *owner = static_cast<DBusMonitorThread *>(user_data);

    if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        Q_EMIT owner->dbusDisconnected();
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    // get base message properties
    DBusMessageObject messageObj;
    messageObj.senderAddress = QString::fromUtf8(dbus_message_get_sender(message));
    messageObj.destinationAddress = QString::fromUtf8(dbus_message_get_destination(message));
    // destinationAddress may be in form of numeric address ":x.y" or in form of bus name "org.kde.xxxx"
    messageObj.type = dbus_message_get_type (message);
    messageObj.typeString = Utils::dbusMessageTypeToString(messageObj.type);

    // handle messages from DBus about new clients
    if (dbus_message_is_method_call(message, DBUS_INTERFACE_DBUS, "Hello")) {
        // new bus client connected
        qCDebug(logMon) << "new client connected:" << messageObj.senderAddress;
        uint namePid = owner->d_ptr->queryBusNameUnixPid(messageObj.senderAddress);
        if (namePid > 0) {
            qCDebug(logMon) << "   new client PID:" << namePid;
            owner->d_ptr->addNamePid(messageObj.senderAddress, namePid);
        } else {
            qCWarning(logMon) << "   failed to query unix PID for new client!";
        }
    }

    // handle messages from DBus about new names
    if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "NameAcquired")) {
        // NameAcquired(STRING name)
        char *str_ptr = nullptr;
        DBusError derror = DBUS_ERROR_INIT;
        if (dbus_message_get_args(message, &derror, DBUS_TYPE_STRING, &str_ptr, DBUS_TYPE_INVALID)) {
            const QString newName = QString::fromUtf8(str_ptr);
            // name may be numeric, if so, no need to resolve it
            if (!Utils::isNumericAddress(newName)) {
                const QString nameOwner = owner->d_ptr->queryNameOwner(newName);
                if (!nameOwner.isEmpty()) {
                    owner->d_ptr->addNameOwner(newName, nameOwner);
                    qCDebug(logMon) << "new name on bus: " << newName << nameOwner;
                }
            }
        }
    }

    // handle messages from DBus about names gone
    if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "NameLost")) {
        // NameLost(STRING name)
        char *str_ptr = nullptr;
        DBusError derror = DBUS_ERROR_INIT;
        if (dbus_message_get_args(message, &derror, DBUS_TYPE_STRING, &str_ptr, DBUS_TYPE_INVALID)) {
            const QString busName = QString::fromUtf8(str_ptr);
            const QString busAddr = QString::fromUtf8(dbus_message_get_destination(message));
            //qCDebug(logMon) << "NameLost:" << busName << busAddr;
            // NameLost: ":1.1319" :1.1319
            // NameLost: "org.dharkael.Flameshot" :1.1225

            // name may be numeric, if so, no need to delete it
            if (!Utils::isNumericAddress(busName)) {
                owner->d_ptr->removeNameOwner(busAddr, busName);
                qCDebug(logMon) << "remove name:" << busName << "from" << busAddr;
            }
        }
    }

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

    // resolve addresses to numeric
    if (!Utils::isNumericAddress(messageObj.senderAddress)) {
        messageObj.senderAddress = owner->d_ptr->resolveNameAddress(messageObj.senderAddress);
    }
    if (!Utils::isNumericAddress(messageObj.destinationAddress)) {
        messageObj.destinationAddress = owner->d_ptr->resolveNameAddress(messageObj.destinationAddress);
    }

    bool thisIsMyMessage = false;
    if ((messageObj.senderAddress == owner->d_ptr->m_myName)
            || (messageObj.destinationAddress == owner->d_ptr->m_myName)) {
        thisIsMyMessage = true;
    }
    if ((messageObj.senderAddress == owner->d_ptr->m_myName2)
            || (messageObj.destinationAddress == owner->d_ptr->m_myName2)) {
        thisIsMyMessage = true;
    }

    messageObj.senderPid = owner->d_ptr->resolvePid(messageObj.senderAddress);
    messageObj.senderNames = owner->d_ptr->resolveDBusAddressToName(messageObj.senderAddress);
    messageObj.destinationPid = owner->d_ptr->resolvePid(messageObj.destinationAddress);
    messageObj.destinationNames = owner->d_ptr->resolveDBusAddressToName(messageObj.destinationAddress);

#ifdef Q_OS_LINUX
    if (messageObj.senderPid > 0) {
        messageObj.senderExe = Utils::pid2filename(messageObj.senderPid);
    }
    if (messageObj.destinationPid > 0) {
        messageObj.destinationExe = Utils::pid2filename(messageObj.destinationPid);
    }
#endif

    // maybe some other processing required
    if (!thisIsMyMessage) {
        // do not show messages from/to monitor itself
        Q_EMIT owner->messageReceived(messageObj);
    }

    // Monitors must not allow libdbus to reply to messages, so we eat the message. See DBus bug 1719.
    return DBUS_HANDLER_RESULT_HANDLED;
}


void DBusMonitorThreadPrivate::run()
{
    m_monitor_active = true;
    Q_EMIT owner->isMonitorActiveChanged();

    while (dbus_connection_read_write_dispatch(m_dconn, 250)) {
        if (owner->isInterruptionRequested()) {
            qCDebug(logMon) << "Interruption requested, breaking DBus loop";
            break;
        }
    }

    closeDbusConn();
    Q_EMIT owner->isMonitorActiveChanged();
}
