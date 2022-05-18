#pragma once

#include "JSON.hpp"
#include "Server.hpp"

Server *loadConfig(const string filename);

int isInteger(double &num);
int isUInteger(double &num);

int getUIntegerField(JSON::Object *src, const std::string &key, int &value);
int getStringField(JSON::Object *src, const std::string &key, std::string &value);
int getBooleanField(JSON::Object *src, const std::string &key, bool &value);

int parseServerBlocks(JSON::Object *src, Server *serv);
int parseServerBlock(JSON::Object *src, ServerBlock &dst);
int parseLocations(JSON::Object *src, std::map<std::string, Location> &res);
int parseLocation(JSON::Object *src, Location &dst);

int parseCGI(JSON::Object *src, std::map<std::string, std::string> &res);
int parseErrorPages(JSON::Object *src, std::map<int, std::string> &res);
int parseCombinedArray(JSON::Object *src, std::string &key, std::vector<std::string> &res);
int parseAllowedMethods(JSON::Object *src, std::vector<std::string> &res);
int parseIndex(JSON::Object *src, std::vector<std::string> &res);