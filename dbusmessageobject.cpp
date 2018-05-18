#include "dbusmessageobject.h"


bool DBusMessageObject::operator==(const DBusMessageObject &o) const
{
    return (type == o.type)
        && (serial == o.serial)
        && (replySerial == o.replySerial)
        && (sender == o.sender)
        && (destination == o.destination)
        && (path == o.path)
        && (interface == o.interface)
        && (member == o.member);
}

bool DBusMessageObject::operator!=(const DBusMessageObject &o) const
{
    return !((*this) == o);
}
