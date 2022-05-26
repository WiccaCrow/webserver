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

int getUInteger(JSON::Object *src, const std::string &key, ExpectedType type, int &res, int def);
int getUInteger(JSON::Object *src, const std::string &key, ExpectedType type, int &res);

int getString(JSON::Object *src, const std::string &key, ExpectedType type, std::string &res, std::string def);
int getString(JSON::Object *src, const std::string &key, ExpectedType type, std::string &res);

int getBoolean(JSON::Object *src, const std::string &key, ExpectedType type, bool &res, bool def);
int getBoolean(JSON::Object *src, const std::string &key, ExpectedType type, bool &res);

int getArray(JSON::Object *src, const std::string &key, ExpectedType type, std::vector<std::string> &res, std::vector<std::string> def);
int getArray(JSON::Object *src, const std::string &key, ExpectedType type, std::vector<std::string> &res);

template <typename T>
int isSubset(std::vector<T> set, std::vector<T> subset);

// Default values
std::vector<std::string> getDefaultAllowedMethods();
std::vector<std::string> getDefaultIndex();

// Object parsing
int parseCGI(JSON::Object *src, std::map<std::string, std::string> &res);
int isValidCGI(std::map<std::string, std::string> &res);

int parseErrorPages(JSON::Object *src, std::map<int, std::string> &res);
int isValidErrorPages(std::map<int, std::string> &res);

int parseLocation(JSON::Object *src, Location &dst, Location &def);
int parseLocations(JSON::Object *src, std::map<std::string, Location> &res, Location &base);

int parseServerBlock(JSON::Object *src, ServerBlock &dst);
int parseServerBlocks(JSON::Object *src, Server *serv);
Server *loadConfig(const string filename);
