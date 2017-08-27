#ifndef MAIN_HPP
#define MAIN_HPP

#include <QtCore/QTextCodec>
#include <QtWidgets/QApplication>
#include <QtCore/QDir>

#include <ipnoise-common/log.h>
#include <ipnoise-common/strings.h>
#include <ipnoise-common/path.hpp>

#include "config.hpp"

#define VERSION "ver. 0.01"

void    usage(int, char **);
int     noGui(Config *a_conf);
int     main(int, char **);

#endif

