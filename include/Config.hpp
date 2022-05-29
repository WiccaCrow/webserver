#pragma once

#include "JSON.hpp"
#include "Server.hpp"

int isInteger(double &num);
int isUInteger(double &num);

enum ExpectedType {
    STRING,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY
};

std::string getDataTypeName(ExpectedType type);
int         typeExpected(JSON::AType *ptr, ExpectedType type);

template <typename T>
int basicCheck(JSON::Object *src, const std::string &key, ExpectedType type, T &res, T def);
int basicCheck(JSON::Object *src, const std::string &key, ExpectedType type);

int getUInteger(JSON::Object *src, const std::string &key, int &res, int def);
int getUInteger(JSON::Object *src, const std::string &key, int &res);

int getString(JSON::Object *src, const std::string &key, std::string &res, std::string def);
int getString(JSON::Object *src, const std::string &key, std::string &res);

int getBoolean(JSON::Object *src, const std::string &key, bool &res, bool def);
int getBoolean(JSON::Object *src, const std::string &key, bool &res);

int getArray(JSON::Object *src, const std::string &key, std::vector<std::string> &res, std::vector<std::string> def);
int getArray(JSON::Object *src, const std::string &key, std::vector<std::string> &res);

template <typename T>
int isSubset(std::vector<T> set, std::vector<T> subset);

// Default values
std::vector<std::string> getDefaultAllowedMethods();

// Object parsing
int parseCGI(JSON::Object *src, std::map<std::string, HTTP::CGI> &res);
int isValidCGI(std::map<std::string, HTTP::CGI> &res);

int parseErrorPages(JSON::Object *src, std::map<int, std::string> &res);
int isValidErrorPages(std::map<int, std::string> &res);

int parseLocation(JSON::Object *src, HTTP::Location &dst, HTTP::Location &def);
int parseLocations(JSON::Object *src, std::map<std::string, HTTP::Location> &res, HTTP::Location &base);

int parseServerBlock(JSON::Object *src, HTTP::ServerBlock &dst);
int parseServerBlocks(JSON::Object *src, Server *serv);

Server *loadConfig(const string filename);
