#ifndef DBUSMESSAGESMODEL_H
#define DBUSMESSAGESMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QByteArray>
#include <QVector>
#include <QMutex>

#include "dbusmessageobject.h"


class DBusMessagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        Serial = Qt::UserRole + 1,
        ReplySerial,
        Type,
        Sender,
        Destination,
        Path,
        Interface,
        Member
    };

public:
    explicit DBusMessagesModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public:
    void addMessage(const DBusMessageObject &dmsg);
    void addMessage(DBusMessageObject &&dmsg);

private:
    QHash<int, QByteArray> m_roles;
    QVector<DBusMessageObject> m_data;
    mutable QMutex m_mutex;
};

#endif // DBUSMESSAGESMODEL_H
