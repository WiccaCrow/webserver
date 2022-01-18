#include <fstream>
#include <iostream>

/**********************
 *  addrFromEtcHosts  *
 **********************
*/
/* Description:
 *      addrFromEtcHosts - finds a line in file "/etc/hosts" containing 
 *      substring servName.
 * Return value:
 *      If the string was found - IP address.
 *      If the string was not found, an empty string.    
*/

std::string addrFromEtcHosts(std::string servName) {
    std::ifstream filename("/etc/hosts", std::ifstream::in);
    std::string   fin_str;
    if (filename.fail())
        return "";
    while (!filename.eof()) {
        getline(filename, fin_str);
        size_t pos = fin_str.find(servName, 0);
        if (pos && (fin_str[pos - 1] == ' ' || fin_str[pos - 1] == '\t') &&
            pos != std::string::npos &&
            ((fin_str[pos + servName.length()] == ' ' ||
              fin_str[pos + servName.length()] == '\t') ||
             fin_str[pos + servName.length()] == 0)) {
            int i = 0;
            for (; !std::isdigit(fin_str[i]); ++i) {
            }
            pos = fin_str.find(" ", i);
            size_t posTab = fin_str.find("\t", i);
            if (pos > posTab)
                pos = posTab;
            return (fin_str.substr(i, pos));
        }
    }
    filename.close();
    return ("");
}

int main() {
    std::cout << fromEtcHosts("localhost") << std::endl;
    return (0);
}

127.0.0.1 hjk fghj localhost ghj 127.0.0.2 hjklocalhost ghj 127.0.0.3 hjk localhos ghj 127.0.0.1 localhost