#pragma once

#include <fstream>
#include <list>
#include <vector>

#include "Server.hpp"
#include "Utils.hpp"

#define OPEN_CTX  '{'
#define CLOSE_CTX '}'
#define COMMENT   ';'

class Conf {
    std::list<std::string> _rawConf;

    std::list<std::string>::iterator curr;

    void *_ctx;
    int   depth = 0;

    public:
    int processKeyword(Server &server, const std::string &keyword);
    int closeContext(Server &server, const std::string &line);

    int read(std::string &filename);
    int load(Server &server, std::string &filename);
    int parse(void);
    int parseLine(void);
};
