#ifndef H_MESSAGECONTENTSPARSER
#define H_MESSAGECONTENTSPARSER

#include <QVariant>
#include <QList>

typedef struct DBusMessageIter DBusMessageIter;

QVariantList parseMessageContents(DBusMessageIter *iter);

#endif
