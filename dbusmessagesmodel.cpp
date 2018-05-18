#include "dbusmessagesmodel.h"

DBusMessagesModel::DBusMessagesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> DBusMessagesModel::roleNames() const
{
    const QHash<int, QByteArray> r = {
        {Serial,       QByteArrayLiteral("serial")},
        {ReplySerial,  QByteArrayLiteral("replySerial")},
        {Type,         QByteArrayLiteral("type")},
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
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}


QVariant DBusMessagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
