#ifndef DBUSMESSAGESMODEL_H
#define DBUSMESSAGESMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QByteArray>


class DBusMessagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        Serial,
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

private:
    QHash<int, QByteArray> m_roles;
};

#endif // DBUSMESSAGESMODEL_H
