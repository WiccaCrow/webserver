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
    # define KW_CGI_METHODS      "cgi_methods"
    # define KW_POST_MAX_BODY    "post_max_body"
    # define KW_REDIRECT         "redirect"
    # define KW_AUTH_BASIC       "auth_basic"
    # define KW_REALM            "realm"
    # define KW_USER_FILE        "user_file"
    # define KW_CODE             "code" 
    # define KW_URL              "url"
    # define KW_PROXY_PASS       "proxy_pass"
    # define KW_PROXY_DOMAINS    "proxy_domains"
    # define KW_ADD_HEADERS      "add_headers"

    # define KW_SETTINGS                 "settings"
    # define KW_MAX_WAIT_CONN            "max_wait_conn"
    # define KW_WORKERS                  "workers"
    # define KW_WORKER_TIMEOUT           "worker_timeout"
    # define KW_MAX_REQUESTS             "max_requests"
    # define KW_MAX_CLIENT_TIMEOUT       "max_client_timeout"
    # define KW_MAX_GATEWAY_TIMEOUT      "max_gateway_timeout"
    # define KW_MAX_URI_LENGTH           "max_uri_length"
    # define KW_MAX_HEADER_FIELD_LENGTH  "max_header_field_length"
    # define KW_BLIND_PROXY              "blind_proxy"
    # define KW_SESSION_LIFETIME         "session_lifetime"
    # define KW_CHUNK_SIZE               "chunk_size"
    # define KW_MAX_REG_FILE_SIZE        "max_reg_file_size"
    # define KW_MAX_RANGE_SIZE           "max_range_size"
    # define KW_COOKIE_HTTP_ONLY         "cookie_http_only"
    # define KW_MAX_REG_UPLOAD_SIZE      "max_reg_upload_size"

#endif

int isInteger(double &num);
int isUInteger(double &num);

std::string getExpectedTypeName(ExpectedType);
int         typeExpected(AType *ptr, ExpectedType);

template <typename T>
ConfStatus basicCheck(Object *, const std::string &key, ExpectedType, T &res, T def);
ConfStatus basicCheck(Object *, const std::string &key, ExpectedType);

template <typename T>
int isSubset(std::vector<T> set, std::vector<T> subset);

// Default values
Location::MethodsVec getDefaultAllowedMethods();

// Object parsing
int parseSize(std::string &s, uint64_t &size);

int parseCGI(Object *, Location::CGIsMap &);
int isValidCGI(Location::CGIsMap &);

int parseErrorPages(Object *, Location::ErrorPagesMap &);
int isValidErrorPages(Location::ErrorPagesMap &);

int parseLocation(Object *, Location &dst, Location &def);
int parseLocations(Object *, ServerBlock::LocationsMap &res, Location &base);

int parseServerBlock(Object *, ServerBlock &);
int parseServerBlocks(Object *, Server::ServersMap &);

Server *loadConfig(const std::string filename);
