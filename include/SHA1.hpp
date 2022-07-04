#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <stdio.h>

class SHA1 {

private:
    uint32_t H[5];
    uint32_t W[80];

    static uint32_t K[4];

    void initH(void);
    void processChunk(void);
    void copyChunk(const uint8_t *str);

public:
    SHA1(void);
    ~SHA1(void);

    std::string hash(const std::string &msg);
};

