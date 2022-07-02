#pragma once

#include "Parser.hpp"
#include "Server.hpp"
#include "Utils.hpp"

using namespace JSON;
using namespace HTTP;

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
    # define KW_LISTEN           "listen"
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
    # define KW_PROXY            "proxy"
    # define KW_PASS             "pass"
    # define KW_DOMAINS          "domains"
#endif

int isInteger(double &num);
int isUInteger(double &num);

std::string getExpectedTypeName(ExpectedType);
int         typeExpected(AType *ptr, ExpectedType);

template <typename T>
ConfStatus basicCheck(Object *, const std::string &key, ExpectedType, T &res, T def);
ConfStatus basicCheck(Object *, const std::string &key, ExpectedType);

int getUInteger(Object *, const std::string &key, int &res, int def);
int getUInteger(Object *, const std::string &key, int &res);

int getString(Object *, const std::string &key, std::string &res, std::string def);
int getString(Object *, const std::string &key, std::string &res);

int getBoolean(Object *, const std::string &key, bool &res, bool def);
int getBoolean(Object *, const std::string &key, bool &res);

int getArray(Object *, const std::string &key, std::vector<std::string> &res, std::vector<std::string> def);
int getArray(Object *, const std::string &key, std::vector<std::string> &res);

template <typename T>
int isSubset(std::vector<T> set, std::vector<T> subset);

// Default values
Location::MethodsVec getDefaultAllowedMethods();

// Object parsing
int parseCGI(Object *, Location::CGIsMap &);
int isValidCGI(Location::CGIsMap &);

int parseErrorPages(Object *, Location::ErrorPagesMap &);
int isValidErrorPages(Location::ErrorPagesMap &);

int parseLocation(Object *, Location &dst, Location &def);
int parseLocations(Object *, ServerBlock::LocationsMap &res, Location &base);

int parseServerBlock(Object *, ServerBlock &);
int parseServerBlocks(Object *, Server::ServersMap &);

Server *loadConfig(const std::string filename);
