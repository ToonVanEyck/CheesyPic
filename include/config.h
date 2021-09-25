#ifndef CONFIG_H
#define CONFIG_H

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <wordexp.h>

#define PL "\033[0;32m"
#ifndef LOG
#define LOG(...) do{fprintf(stderr,"config: ");fprintf(stderr, __VA_ARGS__);}while(0)
#endif

int read_config();

#endif