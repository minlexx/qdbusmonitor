#ifndef DBUSMESSAGEOBJECT_H
#define DBUSMESSAGEOBJECT_H

#include <QObject>
#include <QList>

class DBusMessageObject
{
    Q_GADGET

public:
    DBusMessageObject() = default;
    DBusMessageObject(const DBusMessageObject &) = default;
    DBusMessageObject(DBusMessageObject &&) = default;
    DBusMessageObject &operator=(const DBusMessageObject &) = default;
    DBusMessageObject &operator=(DBusMessageObject &&) = default;
    bool operator==(const DBusMessageObject &o) const;
    bool operator!=(const DBusMessageObject &o) const;

public:
    int     type = 0;
    uint    serial = 0;
    uint    replySerial = 0;
    uint    senderPid = 0;
    uint    destinationPid = 0;
    QString typeString;
    QString senderAddress;
    QStringList senderNames;
    QString senderExe;
    QString destinationAddress;
    QStringList destinationNames;
    QString destinationExe;
    QString path;
    QString interface;
    QString member;
    QString errorName; // only used for error mesages
};

Q_DECLARE_METATYPE(DBusMessageObject)

#endif // DBUSMESSAGEOBJECT_H
