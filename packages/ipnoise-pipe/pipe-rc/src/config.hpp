#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QtCore/QMap>
#include <QtCore/QString>

typedef QMap<QString, QString>                  Config;
typedef QMap<QString, QString>::iterator        ConfigIt;
typedef QMap<QString, QString>::const_iterator  ConfigConstIt;

#endif

