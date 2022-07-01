#include "Base64.hpp"

const char *Base64::encoders = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char Base64::decoders[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 62, 0, 0, 0, 63, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0,
    0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 0, 0, 0, 0, 0
};

const std::string Base64::encode(const std::string &in) {

    std::string res;
    res.reserve((in.length() / 3) * 4);
    for (std::size_t i = 0; i < in.size(); i += 3) {

        char o1 = in[i];
        char o2 = o1 != 0 ? in[i + 1] : 0;
        char o3 = o2 != 0 ? in[i + 2] : 0;
        int line = (o1 << 24) | (o2 << 16) | (o3 << 8);

        int i1 = (line & 0xFC000000) >> 26;
        int i2 = (line & 0x3F00000) >> 20;
        int i3 = (line & 0xFC000) >> 14;
        int i4 = (line & 0x3F00) >> 8;

        res += i1 != 0 ? encoders[i1] : '=';
        res += i2 != 0 ? encoders[i2] : '=';
        res += i3 != 0 ? encoders[i3] : '=';
        res += i4 != 0 ? encoders[i4] : '=';
    }
    return res;
}

const std::string Base64::decode(const std::string &in) {

    std::string res;
    res.reserve((in.length() / 4) * 3);

    for (std::size_t i = 0; i < in.size(); i += 4) {

        if (in[i] & 0x80) {
            // Invalid string
            return "";
        }

        int s1 = decoders[static_cast<int>(in[i])];
        int s2 = decoders[static_cast<int>(in[i + 1])];
        int s3 = decoders[static_cast<int>(in[i + 2])];
        int s4 = decoders[static_cast<int>(in[i + 3])];

        int line = (s1 << 26) | (s2 << 20) | (s3 << 14) | (s4 << 8);
        res += static_cast<char>((line << 0) >> 24);
        res += static_cast<char>((line << 8) >> 24);
        res += static_cast<char>((line << 16) >> 24);
    }
    return res;
}
