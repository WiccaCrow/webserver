#include "SHA1.hpp"

SHA1::SHA1(void) {
    memset(H, 0, 5);
    memset(W, 0, 80);
}

SHA1::~SHA1(void) {}

uint32_t SHA1::K[4] = {
    0x5A827999,
    0x6ED9EBA1,
    0x8F1BBCDC,
    0xCA62C1D6
};

void
SHA1::initH(void) {
    H[0] = 0x67452301;
    H[1] = 0xEFCDAB89;
    H[2] = 0x98BADCFE;
    H[3] = 0x10325476;
    H[4] = 0xC3D2E1F0;
}

static inline uint32_t
shift(int n, uint32_t x) {
    return (x << n) | (x >> (32 - n));
}

static uint32_t
f(size_t t, uint32_t B, uint32_t C, uint32_t D) {
    if (t < 20) {
        return (B & C) | (~B & D);
    } else if (t < 40) {
        return (B ^ C ^ D);
    } else if (t < 60) {
        return (B & C) | (B & D) | (C & D);
    } else {
        return (B ^ C ^ D);
    }
}

void
SHA1::processBlock(void) {
    for (size_t t = 16; t < 80; t++) {
        W[t] = shift(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
    }

    uint32_t A = H[0], B = H[1], C = H[2], D = H[3], E = H[4];

    for (size_t t = 0; t < 80; t++) {
        uint32_t tmp = shift(5, A) + f(t, B, C, D) + E + W[t] + K[t / 20];
        E = D;
        D = C;
        C = shift(30, B);
        B = A;
        A = tmp;
    }

    H[0] += A;
    H[1] += B;
    H[2] += C;
    H[3] += D;
    H[4] += E;
}

void
SHA1::copyBlock(const uint8_t *str) {
    size_t i = 0;
    for (size_t j = 0; j < 16; j++) {    
        unsigned char tmp = str[i] ? str[i] : 0x80;
        W[j] = tmp;
        tmp = (tmp == 0x80) ? 0x0 : str[i + 1] ? str[i + 1] : 0x80;
        W[j] = (W[j] << 8) | tmp;
        tmp = (tmp == 0x80 || tmp == 0x0) ? 0x0 : str[i + 2] ? str[i + 2] : 0x80;
        W[j] = (W[j] << 8) | tmp;
        tmp = (tmp == 0x80 || tmp == 0x0) ? 0x0 : str[i + 3] ? str[i + 3] : 0x80;
        W[j] = (W[j] << 8) | tmp;
        i += 4;

        if (tmp == 0x0 || tmp == 0x80)
            break ;
    }
}

std::string
SHA1::hash(const std::string &msg) {

    initH();
    uint64_t bits = static_cast<uint64_t>(msg.length() * 8);
    const uint8_t *str = reinterpret_cast<const uint8_t *>(msg.c_str());

    size_t i = 0;
    const size_t chunksCount = (msg.length() / 64) + 1;
    for (size_t index = 1; index < chunksCount; index++) {
        copyBlock((str + i));
        i += 64;
        processBlock();
    }

    memset((void *)W, 0, 64);
    copyBlock((str + i));
    W[14] = static_cast<uint32_t>((bits >> 32));
    W[15] = static_cast<uint32_t>((bits << 32) >> 32);    
    processBlock();

    char hex[50] = {0};
    sprintf(hex, "%08x%08x%08x%08x%08x", H[0], H[1], H[2], H[3], H[4]);
    return hex;
}
