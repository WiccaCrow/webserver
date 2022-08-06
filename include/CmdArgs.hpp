#pragma once

#include "Utils.hpp"
#include "HTML.hpp"
#include "Logger.hpp"
#include "Globals.hpp"

extern bool isDaemon;

void printUsage(void);
int parseLogLevel(const char *);
int parseLogDirectory(const char *);
int parseArgs(char **);
