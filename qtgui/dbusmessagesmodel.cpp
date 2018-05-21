#include "dbusmessagesmodel.h"

DBusMessagesModel::DBusMessagesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_mutex(QMutex::Recursive)
{
}

QHash<int, QByteArray> DBusMessagesModel::roleNames() const
{
    static const QHash<int, QByteArray> r = {
        {Serial,       QByteArrayLiteral("serial")},
        {ReplySerial,  QByteArrayLiteral("replySerial")},
        {Type,         QByteArrayLiteral("type")},
        {TypeString,   QByteArrayLiteral("typeString")},
        {Sender,       QByteArrayLiteral("sender")},
        {Destination,  QByteArrayLiteral("destination")},
        {Path,         QByteArrayLiteral("path")},
        {Interface,    QByteArrayLiteral("interface")},
        {Member,       QByteArrayLiteral("member")},
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
    case Role::Serial:        ret = dmsg.serial;       break;
    case Role::ReplySerial:   ret = dmsg.replySerial;  break;
    case Role::Type:          ret = dmsg.type;         break;
    case Role::TypeString:    ret = dmsg.typeString;   break;
    case Role::Sender:        ret = dmsg.sender;       break;
    case Role::Destination:   ret = dmsg.destination;  break;
    case Role::Path:          ret = dmsg.path;         break;
    case Role::Interface:     ret = dmsg.interface;    break;
    case Role::Member:        ret = dmsg.member;       break;
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
