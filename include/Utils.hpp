#pragma once

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

std::string itoh(int nb);
void trim(std::string &s, const char *t);
void toLowerCase(std::string &s);
void skipSpaces(const std::string &line, size_t &pos);

std::string getWord(const std::string &line, char delimiter, size_t &pos);
std::string getWord_wicca(const std::string &line, char delimiter, size_t &pos); // wicca

#if __cplusplus < 201103L

#include <stdio.h>

#include <string>

std::string to_string(int val);

#endif

int		isFile(const std::string& fileName);
