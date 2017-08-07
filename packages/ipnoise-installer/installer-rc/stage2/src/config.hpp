#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QMap>
#include <QString>

typedef QMap<QString, QString>                  Config;
typedef QMap<QString, QString>::iterator        ConfigIt;
typedef QMap<QString, QString>::const_iterator  ConfigConstIt;

#endif

