#pragma once

#include "JSON.hpp"
#include "Server.hpp"

enum ExpectedType {
    STRING,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY
};

enum ConfStatus {
    NONE_OR_INV = 0,
    SET,
    DEFAULT
};

#ifndef CONFIG_KEYWORDS
    # define CONFIG_KEYWORDS
    # define KW_SERVERS          "servers"
    # define KW_ADDR             "addr"
    # define KW_PORT             "port"
    # define KW_SERVER_NAMES     "server_names"
    # define KW_ERROR_PAGES      "error_pages"
    # define KW_LOCATIONS        "locations"
    # define KW_CGI              "cgi"
    # define KW_ROOT             "root"
    # define KW_ALIAS            "alias"
    # define KW_INDEX            "index"
    # define KW_AUTOINDEX        "autoindex"
    # define KW_METHODS_ALLOWED  "methods_allowed"
    # define KW_POST_MAX_BODY    "post_max_body"
    # define KW_REDIRECT         "redirect"
    # define KW_AUTH_BASIC       "auth_basic"
    # define KW_REALM            "realm"
    # define KW_USER_FILE        "user_file"
    # define KW_CODE             "code" 
    # define KW_URL              "url"
#endif

int isInteger(double &num);
int isUInteger(double &num);

std::string getExpectedTypeName(ExpectedType type);
int         typeExpected(JSON::AType *ptr, ExpectedType type);

template <typename T>
ConfStatus basicCheck(JSON::Object *src, const std::string &key, ExpectedType type, T &res, T def);
ConfStatus basicCheck(JSON::Object *src, const std::string &key, ExpectedType type);

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
