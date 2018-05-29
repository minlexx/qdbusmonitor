#include "dbusmessagesmodel.h"

DBusMessagesModel::DBusMessagesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_mutex(QMutex::Recursive)
{
}

QHash<int, QByteArray> DBusMessagesModel::roleNames() const
{
    static const QHash<int, QByteArray> r = {
        {Serial,             QByteArrayLiteral("serial")},
        {ReplySerial,        QByteArrayLiteral("replySerial")},
        {Timestamp,          QByteArrayLiteral("timestamp")},
        {Type,               QByteArrayLiteral("type")},
        {TypeString,         QByteArrayLiteral("typeString")},
        {SenderAddress,      QByteArrayLiteral("senderAddress")},
        {SenderNames,        QByteArrayLiteral("senderNames")},
        {SenderPid,          QByteArrayLiteral("senderPid")},
        {SenderExe,          QByteArrayLiteral("senderExe")},
        {DestinationAddress, QByteArrayLiteral("destinationAddress")},
        {DestinationNames,   QByteArrayLiteral("destinationNames")},
        {DestinationPid,     QByteArrayLiteral("destinationPid")},
        {DestinationExe,     QByteArrayLiteral("destinationExe")},
        {Path,               QByteArrayLiteral("path")},
        {Interface,          QByteArrayLiteral("interface")},
        {Member,             QByteArrayLiteral("member")},
    };
    return r;
}

int DBusMessagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    QMutexLocker guard(&m_mutex);
    return m_data.size();
}


QVariant DBusMessagesModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if (!index.isValid()) {
        return ret;
    }

    int row = index.row();
    QMutexLocker guard(&m_mutex);
    if ((row < 0) || (row >= m_data.size())) {
        return ret;
    }

    const DBusMessageObject &dmsg = m_data.at(row);
    switch (role) {
    case Role::Serial:             ret = dmsg.serial;             break;
    case Role::ReplySerial:        ret = dmsg.replySerial;        break;
    case Role::Timestamp:          ret = dmsg.timestamp;          break;
    case Role::Type:               ret = dmsg.type;               break;
    case Role::TypeString:         ret = dmsg.typeString;         break;
    case Role::SenderAddress:      ret = dmsg.senderAddress;      break;
    case Role::SenderNames:        ret = dmsg.senderNames;        break;
    case Role::SenderPid:          ret = dmsg.senderPid;          break;
    case Role::SenderExe:          ret = dmsg.senderExe;          break;
    case Role::DestinationAddress: ret = dmsg.destinationAddress; break;
    case Role::DestinationNames:   ret = dmsg.destinationNames;   break;
    case Role::DestinationPid:     ret = dmsg.destinationPid;     break;
    case Role::DestinationExe:     ret = dmsg.destinationExe;     break;
    case Role::Path:               ret = dmsg.path;               break;
    case Role::Interface:          ret = dmsg.interface;          break;
    case Role::Member:             ret = dmsg.member;             break;
    }
    return ret;
}

void DBusMessagesModel::addMessage(const DBusMessageObject &dmsg)
{
    QMutexLocker guard(&m_mutex);
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(dmsg);
    endInsertRows();
}

void DBusMessagesModel::addMessage(DBusMessageObject &&dmsg)
{
    QMutexLocker guard(&m_mutex);
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(dmsg);
    endInsertRows();
}

void DBusMessagesModel::clear()
{
    QMutexLocker guard(&m_mutex);
    beginResetModel();
    m_data.clear();
    endResetModel();
}

int DBusMessagesModel::findSerial(uint serial) const
{
    QMutexLocker guard(&m_mutex);
    for (int idx = 0; idx < m_data.size(); idx++) {
        const auto &msg = m_data.at(idx);
        if (msg.serial == serial) {
            return idx;
        }
    }
    return -1;
}

int DBusMessagesModel::findReplySerial(uint serial) const
{
    QMutexLocker guard(&m_mutex);
    for (int idx = 0; idx < m_data.size(); idx++) {
        const auto &msg = m_data.at(idx);
        if (msg.replySerial == serial) {
            return idx;
        }
    }
    return -1;
}
