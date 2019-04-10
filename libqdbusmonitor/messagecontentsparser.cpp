#include "messagecontentsparser.h"
#include <QMetaType>
#include <QLoggingCategory>

#include <dbus/dbus.h>
#include <stdio.h>
#include <inttypes.h>
#ifdef Q_OS_LINUX
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

Q_LOGGING_CATEGORY(logMessageParser, "monitor.messageparser")


static QString print_hex(const unsigned char *bytes, unsigned int len, int depth)
{
    QString ret;
    unsigned int i, columns;
    /* Each byte takes 3 cells (two hexits, and a space), except the last one. */
    const unsigned CELLS_PER_BYTE = 3;
    const unsigned INDENT = 3;

    ret.append(QLatin1String("array of bytes [\n"));
    columns = (80 - ((static_cast<unsigned>(depth) + 1) * INDENT)) / CELLS_PER_BYTE;

    if (columns < 8) {
        columns = 8;
    }

    i = 0;
    while (i < len) {
        ret.append(QString::number(static_cast<uint>(bytes[i]), 16));
        i++;

        if (i != len) {
            if (i % columns == 0) {
                ret.append(QLatin1String("\n"));
            } else {
                ret.append(QLatin1String(" "));
            }
        }
    }

    ret.append(QLatin1String("\n"));
    ret.append(QLatin1String("]\n"));
    return ret;
}

static QString print_ay_str(DBusMessageIter *iter, int depth)
{
    /* True if every byte in the bytestring (so far) is printable
     * ASCII, with one exception: the last byte is also allowed to be \0. */
    dbus_bool_t all_ascii = TRUE;
    const unsigned char *bytes = nullptr;
    int len = 0;
    int i = 0;
    QString ret;
    QVariantList retArr;

    dbus_message_iter_get_fixed_array(iter, &bytes, &len);

    for (i = 0; i < len; i++) {
        if ((bytes[i] < 32 || bytes[i] > 126) &&
                (i < len - 1 || bytes[i] != '\0')) {
            all_ascii = FALSE;
            break;
        }
    }

    qCDebug(logMessageParser) << depth << "print_ay: array size:" << len
                              << "all_ascii:" << all_ascii;

    if (all_ascii && (len > 0) && (bytes[len - 1] == '\0')) {
        /* zero-terminated */
        ret = QStringLiteral("array of bytes \"");
        ret.append(QString::fromUtf8(reinterpret_cast<const char *>(bytes)));
        ret.append(QLatin1String("\" + \\0"));
    } else if (all_ascii) {
        /* not zero-terminated */
        unsigned char *copy = reinterpret_cast<unsigned char *>(dbus_malloc(static_cast<size_t>(len) + 1));
        if (copy) {
            memcpy(copy, bytes, static_cast<size_t>(len));
            copy[len] = '\0';
            ret = QStringLiteral("array of bytes \"");
            ret.append(QString::fromUtf8(reinterpret_cast<const char *>(copy)));
            ret.append(QLatin1String("\""));
            dbus_free(copy);
        }
    } else {
        ret = print_hex(bytes, static_cast<unsigned>(len), depth);
    }
    return ret;
}

#ifdef Q_OS_LINUX
static QVariant print_fd (int fd, int depth)
{
    QString strRet;
    int ret;
    struct stat statbuf;
    union {
        struct sockaddr sa;
        struct sockaddr_storage storage;
        struct sockaddr_un un;
        struct sockaddr_in ipv4;
        struct sockaddr_in6 ipv6;
    } addr, peer;
    char hostip[INET6_ADDRSTRLEN];
    socklen_t addrlen = sizeof(addr);
    socklen_t peerlen = sizeof(peer);
    int has_peer;

    Q_UNUSED(depth)

    memset(&statbuf, 0, sizeof(statbuf));

    /* Don't print the fd number: it is different in every process and since
     * dbus-monitor closes the fd after reading it, the same number would be
     * printed again and again.
     */
    strRet.append(QLatin1String("file descriptor"));
    if (fd == -1) {
        return QVariant(strRet);
    }

    ret = fstat(fd, &statbuf);
    if (ret == -1) {
        return QVariant(strRet);
    }

    strRet.append(QStringLiteral(" inode: %1").arg(static_cast<int>(statbuf.st_ino)));

    strRet.append(QLatin1String("; type: "));
    if (S_ISREG(statbuf.st_mode)) strRet.append(QLatin1String("file"));
    if (S_ISDIR(statbuf.st_mode)) strRet.append(QLatin1String("directory"));
    if (S_ISCHR(statbuf.st_mode)) strRet.append(QLatin1String("char"));
    if (S_ISBLK(statbuf.st_mode)) strRet.append(QLatin1String("block"));
    if (S_ISFIFO(statbuf.st_mode)) strRet.append(QLatin1String("fifo"));
    if (S_ISLNK(statbuf.st_mode)) strRet.append(QLatin1String("link"));
    if (S_ISSOCK(statbuf.st_mode)) strRet.append(QLatin1String("socket"));

    /* If it's not a socket, getsockname will just return -1 with errno ENOTSOCK. */

    memset(&addr, 0, sizeof (addr));
    memset(&peer, 0, sizeof (peer));

    if (getsockname(fd, &addr.sa, &addrlen)) {
        return QVariant(strRet);
    }

    has_peer = !getpeername(fd, &peer.sa, &peerlen);

    strRet.append(QLatin1String("; address family: "));
    switch (addr.sa.sa_family)
    {
    case AF_UNIX:
        strRet.append(QLatin1String("unix "));
        if (addr.un.sun_path[0] == '\0') {
            /* Abstract socket might not be zero-terminated and length is
             * variable. Who designed this interface?
             * Write the name in the same way as /proc/net/unix
             * See manual page unix(7)
             */
            char unix_fd_path[sizeof(addr.un.sun_path) + 2];
            memset(unix_fd_path, 0, sizeof(unix_fd_path));
            memcpy(unix_fd_path, addr.un.sun_path + 1, sizeof(addr.un.sun_path) - 1);
            strRet.append(QLatin1String("name @"));
            strRet.append(QString::fromUtf8(unix_fd_path));

            if (has_peer) {
                memset(unix_fd_path, 0, sizeof(unix_fd_path));
                memcpy(unix_fd_path, addr.un.sun_path + 1, sizeof(addr.un.sun_path) - 1);
                strRet.append(QLatin1String("peer @"));
                strRet.append(QString::fromUtf8(unix_fd_path));
            }
        } else {
            strRet.append(QLatin1String("name "));
            strRet.append(QString::fromUtf8(addr.un.sun_path));
            if (has_peer) {
                strRet.append(QLatin1String(" peer "));
                strRet.append(QString::fromUtf8(peer.un.sun_path));
            }
        }
        break;

    case AF_INET:
        strRet.append(QLatin1String("inet "));
        if (inet_ntop(AF_INET, &addr.ipv4.sin_addr, hostip, sizeof (hostip))) {
            strRet.append(QStringLiteral("name %1 port %2")
                          .arg(QString::fromUtf8(hostip))
                          .arg(ntohs(addr.ipv4.sin_port)));
        }
        if (has_peer && inet_ntop(AF_INET, &peer.ipv4.sin_addr, hostip, sizeof (hostip))) {
            strRet.append(QStringLiteral(" peer %1 port %2")
                          .arg(QString::fromUtf8(hostip))
                          .arg(ntohs(peer.ipv4.sin_port)));
        }
        break;

#ifdef AF_INET6
    case AF_INET6:
        strRet.append(QLatin1String("inet6 "));
        if (inet_ntop(AF_INET6, &addr.ipv6.sin6_addr, hostip, sizeof (hostip))) {
            strRet.append(QStringLiteral("name %1 port %2")
                          .arg(QString::fromUtf8(hostip))
                          .arg(ntohs(addr.ipv6.sin6_port)));
        }
        if (has_peer && inet_ntop(AF_INET6, &peer.ipv6.sin6_addr, hostip, sizeof (hostip))) {
            strRet.append(QStringLiteral(" peer %1 port %2")
                          .arg(QString::fromUtf8(hostip))
                          .arg(ntohs(peer.ipv6.sin6_port)));
        }
        break;
#endif

#ifdef AF_BLUETOOTH
    case AF_BLUETOOTH:
        strRet.append(QLatin1String("bluetooth "));
        break;
#endif

    default:
        strRet.append(QStringLiteral("unknown family (%d)").arg(static_cast<int>(addr.sa.sa_family)));
        break;
    }

    return QVariant(strRet);
}
#endif

static QVariant print_iter(DBusMessageIter *iter, dbus_bool_t literal, int depth)
{
    QVariantList ret;

    do
    {
        QVariant varArgument;
        int type = dbus_message_iter_get_arg_type(iter);

        if (type == DBUS_TYPE_INVALID) {
            // qCWarning(logMessageParser) << "print_iter: got DBUS_TYPE_INVALID!";
            // ^^ this is normal
            break;
        }

        switch (type) {
        case DBUS_TYPE_STRING:
        {
            char *val;
            QString arg;
            dbus_message_iter_get_basic(iter, &val);
            if (!literal) {
                arg.append(QLatin1String("string \""));
            }
            arg.append(QString::fromUtf8(val));
            if (!literal) {
                arg.append(QLatin1String("\"")); // do not append newlines
            }
            varArgument = arg;
            break;
        }

        case DBUS_TYPE_SIGNATURE:
        {
            char *val;
            QString arg;
            dbus_message_iter_get_basic(iter, &val);
            if (!literal) {
                arg.append(QLatin1String("signature \""));
            }
            arg.append(QString::fromUtf8(val));
            if (!literal) {
                arg.append(QLatin1String("\"")); // do not append newlines
            }
            varArgument = arg;
            break;
        }

        case DBUS_TYPE_OBJECT_PATH:
        {
            char *val;
            QString arg;
            dbus_message_iter_get_basic(iter, &val);
            if (!literal) {
                arg.append(QLatin1String("object path \""));
            }
            arg.append(QString::fromUtf8(val));
            if (!literal) {
                arg.append(QLatin1String("string \"")); // do not append newlines
            }
            varArgument = arg;
            break;
        }

        case DBUS_TYPE_INT16:
        {
            dbus_int16_t val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(static_cast<int>(val));
            break;
        }

        case DBUS_TYPE_UINT16:
        {
            dbus_uint16_t val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(static_cast<uint>(val));
            break;
        }

        case DBUS_TYPE_INT32:
        {
            dbus_int32_t val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(static_cast<int>(val));
            break;
        }

        case DBUS_TYPE_UINT32:
        {
            dbus_uint32_t val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(static_cast<uint>(val));
            break;
        }

        case DBUS_TYPE_INT64:
        {
            dbus_int64_t val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(static_cast<qint64>(val));
            break;
        }

        case DBUS_TYPE_UINT64:
        {
            dbus_uint64_t val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(static_cast<quint64>(val));
            break;
        }

        case DBUS_TYPE_DOUBLE:
        {
            double val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(val);
            break;
        }

        case DBUS_TYPE_BYTE:
        {
            unsigned char val;
            dbus_message_iter_get_basic(iter, &val);
            varArgument = QVariant(static_cast<uint>(val));
            break;
        }

        case DBUS_TYPE_BOOLEAN:
        {
            dbus_bool_t val;
            dbus_message_iter_get_basic(iter, &val);
            if (val) {
                varArgument = QVariant(true);
            } else {
                varArgument = QVariant(false);
            }
            break;
        }

        case DBUS_TYPE_VARIANT:
        {
            DBusMessageIter subiter;
            // QString arg;
            dbus_message_iter_recurse (iter, &subiter);
            // arg = QStringLiteral("variant: ");
            // arg.append(print_iter(&subiter, literal, depth + 1).toString());
            // varArgument = arg;
            varArgument = print_iter(&subiter, literal, depth + 1); // or maybe this way
            break;
        }
        case DBUS_TYPE_ARRAY:
        {
            int current_type;
            DBusMessageIter subiter;
            //QString arg;
            QVariantList array;

            dbus_message_iter_recurse (iter, &subiter);

            current_type = dbus_message_iter_get_arg_type(&subiter);

            // no special case for array of bytes
            //if (current_type == DBUS_TYPE_BYTE) {
            //    array = print_ay(&subiter, depth);
            //    varArgument = QVariant::fromValue<QVariantList>(array);
            //    // qCDebug(logMessageParser) << "after print_ay, returned value is: " << varArgument;
            //    break;
            //}

            //arg = QStringLiteral("array [");
            while (current_type != DBUS_TYPE_INVALID) {
                //arg.append(print_iter(&subiter, literal, depth + 1).toString());
                QVariant recursedArray = print_iter(&subiter, literal, depth + 1);
                if (recursedArray.canConvert(QMetaType::QVariantList)) {
                    array = recursedArray.value<QVariantList>();
                }

                dbus_message_iter_next (&subiter);
                current_type = dbus_message_iter_get_arg_type(&subiter);

                //if (current_type != DBUS_TYPE_INVALID) {
                //    arg.append(QLatin1String(", "));
                //}
            }
            //arg.append(QLatin1String("]"));
            //varArgument = arg;
            varArgument = QVariant::fromValue<QVariantList>(array);
            break;
        }
        case DBUS_TYPE_DICT_ENTRY:
        {
            DBusMessageIter subiter;
            // QString arg;
            QVariantMap mapp;
            dbus_message_iter_recurse(iter, &subiter);

            const QVariant mapKey = print_iter(&subiter, literal, depth + 1);
            dbus_message_iter_next(&subiter);
            const QVariant mapValue = print_iter(&subiter, literal, depth + 1);
            mapp.insert(mapKey.toString(), mapValue);
            qCDebug(logMessageParser) << "dict: " << mapKey << mapKey.type() << "=" << mapValue;
            varArgument = QVariant::fromValue<QVariantMap>(mapp);
            break;
        }

        case DBUS_TYPE_STRUCT:
        {
            int current_type;
            DBusMessageIter subiter;
            QVariantList structEntryList;

            dbus_message_iter_recurse (iter, &subiter);

            // arg = QStringLiteral("struct {\n");
            while ((current_type = dbus_message_iter_get_arg_type (&subiter)) != DBUS_TYPE_INVALID) {
                QVariant structField = print_iter(&subiter, literal, depth + 1);
                //qCDebug(logMessageParser) << "struct: element = " << structField;
                // arg.append(struct_field.toString());
                structEntryList.append(structField);
                dbus_message_iter_next(&subiter);
                //if (dbus_message_iter_get_arg_type(&subiter) != DBUS_TYPE_INVALID) {
                //    //arg.append(QLatin1String(", "));
                //    qCDebug(logMessageParser) << ", ";
                //}
            }
            // arg.append(QLatin1String("}"));
            // varArgument = arg;
            varArgument = structEntryList;
            break;
        }

#ifdef Q_OS_LINUX
        case DBUS_TYPE_UNIX_FD:
        {
            int fd;
            dbus_message_iter_get_basic(iter, &fd);

            varArgument = print_fd(fd, depth + 1);

            /* dbus_message_iter_get_basic() duplicated the fd, we need to
             * close it after use. The original fd will be closed when the
             * DBusMessage is released.
             */
            close (fd);
            break;
        }
#endif

        default:
            qCWarning(logMessageParser) << depth << "too dumb to decipher argument type:" << type;
            break;
        }

        if (varArgument.isValid()) {
            ret.append(varArgument);
        }
    } while (dbus_message_iter_next(iter));

    // calculate return value
    if (ret.isEmpty()) {
        // just return invalid qvariant
        return QVariant();
    }

    // check if return list consists of only 1 item - return only it
    if (ret.size() == 1) {
        // qCDebug(logMessageParser) << "returning single element" << ret.at(0).type() << ret.at(0);
        return ret.at(0);
    }

    // otherwise return the whole list
    return QVariant::fromValue<QVariantList>(ret);
}


QVariantList parseMessageContents(DBusMessageIter *iter)
{
    QVariant arg = print_iter(iter, TRUE, 1);
    QVariantList ret;
    if (arg.canConvert(QMetaType::QVariantList)) {
        ret = arg.toList();
    } else {
        ret.append(arg);
    }
    return ret;
}
