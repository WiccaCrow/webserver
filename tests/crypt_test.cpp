#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <pwd.h>
#include <crypt.h>

void encryptstring(const std::string line, const std::string s) {
    
    char salt[2] = {0};
    salt[0] = s[0];
    salt[1] = s[1];
    std::string pwd = crypt(line.c_str, salt);
    std::cout << pwd << std::endl;
}

int main() {
    encryptstring("qweqweqwe", "LS");
    return 0;
}