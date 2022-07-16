#pragma once

#include <cstring>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include "Time.hpp"
#include "SHA1.hpp"

std::string getRandId(void);

std::string join(std::vector<std::string> &v, const std::string &delim = ", ");
std::vector<std::string> split(const std::string &source, const std::string &delimiters = " ");
std::string              getWord(const std::string &line, const char *delims, std::size_t &pos);

std::string &rtrim(std::string &s, const char *t);
std::string &ltrim(std::string &s, const char *t);

void trim(std::string &s, const char *t);
void toLowerCase(std::string &s);
void skipSpaces(const std::string &line, std::size_t &pos);

bool startsWith(const std::string &str, const std::string &beg);
bool endsWith(const std::string &str, const std::string &end);

#if __cplusplus < 201103L

#include <stdio.h>
#include <string>

std::string ulltos(unsigned long long val);
std::string ultos(unsigned long val);
std::string lltos(long long val);
std::string ltos(long val);
std::string itos(int val);
std::string sztos(std::size_t val);

#endif

void writeFile(const std::string &file, const std::string &content);
std::string readFile(const std::string &file);

std::string itohs(int nb);
bool stoll(long long &num, char const *s);
bool stoll(char const *s);

template <typename T>
bool stoi64(T &num, const std::string &s) {
    char              c;
    std::stringstream ss(s.c_str());
    ss >> num;
    return !(ss.fail() || ss.get(c));
}

template <typename T>
std::string NumberToString(T &num) {
    std::ostringstream ss;
    ss << num;
    return ss.str();
}

std::size_t strlen_u8(const std::string &s);

time_t getModifiedTime(const std::string &file);

// RFC validation

extern const char * validMethods[];
bool isValidProtocol(const std::string &protocol);
bool isValidMethod(const std::string &s);
bool isValidPath(const std::string &s);
bool isValidIpv4(const std::string &s);
bool isValidIpv6(const std::string &s);
bool isValidIp(const std::string &ip);
bool isValidPort(const std::string &s);
bool isValidHost(const std::string &hostname);
bool isValidPath(const std::string &path);

// File-related functions
bool isExtension(const std::string &ext);
bool resourceExists(const std::string &filename);
bool isFile(const std::string &filename);
bool isDirectory(const std::string &dirname);
bool isWritableFile(const std::string &filename);
bool isReadableFile(const std::string &filename);
bool isExecutableFile(const std::string &filename);
bool checkRegFilePerms(const std::string &filename, int perm);
int  rmdirNonEmpty(std::string &resourceDel);
