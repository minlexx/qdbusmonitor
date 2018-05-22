#include "dbusmessageobject.h"


bool DBusMessageObject::operator==(const DBusMessageObject &o) const
{
    return (type == o.type)
            && (serial == o.serial)
            && (replySerial == o.replySerial)
            && (senderPid == o.senderPid)
            && (destinationPid == o.destinationPid)
            && (typeString == o.typeString)
            && (senderAddress == o.senderAddress)
            && (senderName == o.senderName)
            && (senderExe == o.senderExe)
            && (destinationAddress == o.destinationAddress)
            && (destinationName == o.destinationName)
            && (destinationExe == o.destinationExe)
            && (path == o.path)
            && (interface == o.interface)
            && (member == o.member)
            && (errorName == o.errorName);
}

bool DBusMessageObject::operator!=(const DBusMessageObject &o) const
{
    return !((*this) == o);
}
