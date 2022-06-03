#pragma once

#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>

void        trim(std::string &s, const char *t);

std::string itoh(int nb);
std::string getWord(const std::string &line, char delimiter, size_t &pos);

void        toLowerCase(std::string &s);
void        skipSpaces(const std::string &line, size_t &pos);

#if __cplusplus < 201103L

    #include <stdio.h>
    #include <string>

std::string to_string(unsigned long val);

#endif

bool isValidIp(const std::string &ip);
bool isValidHost(const std::string &hostname);
bool isValidPath(const std::string &path);
bool isExtension(const std::string &ext);
bool endsWith(const std::string &str, const std::string &end);
bool resourceExists(const std::string &filename);
bool isFile(const std::string &filename);
bool isDirectory(const std::string &dirname);
bool isReadableFile(const std::string &filename);
bool isExecutableFile(const std::string &filename);
bool checkRegFilePerms(const std::string &filename, int perm);
int  rmdirNonEmpty(std::string &resourceDel);
