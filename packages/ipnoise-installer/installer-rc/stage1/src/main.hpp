#ifndef MAIN_HPP
#define MAIN_HPP

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <getopt.h>
#include <time.h>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <ipnoise-common/log.h>
#include <ipnoise-common/strings.h>
#include <ipnoise-common/path.hpp>

#include "config.hpp"
#include "ipnoise-packer/packer/src/packer.hpp"

#define VERSION "ver. 0.01"

void    usage(int, char **);
int     process(Config *a_conf);
int     main(int, char **);

#endif

