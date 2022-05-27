#include "string"
#include "iostream"

bool endsWith(const std::string &str, const std::string &end) {
    if (end.length() >= str.length()) 
        return false;
    return str.find(end, str.length() - end.length()) != std::string::npos;
    // return str.substr(str.length() - end.length()) == end;
}

int main(int ac, char **av) {
    std::string s = ".php";
    std::string ext = ".php";
    std::cout << endsWith(s, ext) << std::endl;
    return 0;
}