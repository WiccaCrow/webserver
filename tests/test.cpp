#include "string"
#include "iostream"

int main(int ac, char **av) {
    std::string s = "\"b\r\n\"\r\n";

    size_t pos = s.find("\r\n");
    std::cout << "pos = " << pos << std::endl;
    std::cout << "len = " << s.length() << std::endl;
    return 0;
}