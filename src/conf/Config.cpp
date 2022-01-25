#include "Config.hpp"

const char  tokensProto[] = {OPEN_CTX, CLOSE_CTX, COMMENT};
const char *keywordsProto[] = {
    "server",
    "index",
    "autoindex",
    "location",
    "root",
    "",
};

const size_t keywordsCount = sizeof(keywordsProto) / sizeof(keywordsProto[0]);

const std::vector<char>   tokens(tokensProto, tokensProto + sizeof(tokensProto));
const std::vector<char *> keywords(keywordsProto, keywordsProto + keywordsCount);

int Conf::processKeyword(Server &server, const std::string &keyword) {
    if (keywords == "server") {
        ServerBlock serverBlock;

        server.addServerBlocks(serverBlock);
    }
}

int Conf::closeContext(Server &server, const std::string &line) {
    if (line.length() != 1)
        return 2;
    depth--;
    return 0;
}

int Conf::parseLine() {
    std::string &line = *curr;

    // 0. Skip if empty
    if (line == "")
        return 0;

    // 1. Trim ends
    trim(line, " \t");

    std::vector<char>::const_iterator begToken;
    std::vector<char>::const_iterator endToken;
    begToken = std::find(tokens.begin(), tokens.end(), line[0]);
    endToken = std::find(tokens.begin(), tokens.end(), line[line.length() - 1]);
    // 2. Check if one of the tokens appeared on the one side of the line
    //if (begToken == tokens.end() && endToken == tokens.end()) {
    //    return 1;
    //}

    // 3. Check if trimmed line starts with a keyword or with an end token
    if (begToken != tokens.end()) {
        switch (line[0]) {
            case CLOSE_CTX:
                return closeContext(server, line);
            case COMMENT:
                // Line commented
                return 0;
            default: {
                break;
            }
        }
    }

    switch (line[line.length() - 1]) {
        case OPEN_CTX:
            depth++;
            break;
        case COMMENT:
            break;
        default:
            // Invalid syntax
            return 2;
    }

    if (!isalpha(line[0])) {
        // Keyword started with non-alpha character
        return 3;
    } else {
        size_t      i = 0;
        std::string keyword = getWord(line, ' ', i);
        for (i = 0; i < keywords.size(); i++) {
            // Need to replace with exact match of keywords
            if (keyword.find(keywords[i]) == 0) {
                break;
            }
        }
        if (i == keywords.size()) {
            // Unknown keyword
            return 4;
        }
        // 4. Process a line
        return processKeyword(server, line);
    }
}

int Conf::read(std::string &filename) {
    std::ifstream in;

    in.open(filename.c_str());

    if (!in.is_open()) {
        Log.error("Cannot load file " + filename);
        return 1;
    }

    std::string line;
    while (!in.eof()) {
        std::getline(in, line, '\n');
        _rawConf.push_back(line);
    }
    return 0;
}

int Conf::load(Server &server, std::string &filename) {
    _ctx = &server;

    if (read(filename)) {
        exit(1);
    }

    if (parse()) {
        // Maybe other
        exit(1);
    }

    if (depth != 0) {
        Log.error("Invalid config syntax");
        exit(2);
    }

    return 0;
}
