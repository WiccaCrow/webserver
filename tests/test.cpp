#include <string>
#include <iostream>

#include "URI.hpp"

int main(int ac, char **av) {
    if (ac < 2) {
        std::cout << "Err: nb argv" << std::endl;
        return 1;
    }

    std::cout << av[1] << std::endl;
    const std::string &s = HTTP::URI::encode(av[1]);
    std::cout << s << std::endl;
    std::cout << HTTP::URI::decode(s) << std::endl;
    return 0;
}