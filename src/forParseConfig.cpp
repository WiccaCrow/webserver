#include <iostream>
#include <fstream>

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

std::string    addrFromEtcHosts(std::string servName) {
    std::ifstream  filename ("/etc/hosts", std::ifstream::in);
    std::string    fin_str;
    while (!filename.eof())
    {
        getline(filename, fin_str);
        size_t pos = fin_str.find(servName, 0);
        if (pos && fin_str[pos - 1] == ' ' && pos != std::string::npos && 
            (fin_str[pos + servName.length()] == ' ' ||
             fin_str[pos + servName.length()] == 0)) {
            int i = 0;
            for (; !std::isdigit(fin_str[i]); ++i) {
            }
            pos = fin_str.find(" ", i);
            return (fin_str.substr(i, pos));
        }
    }
    filename.close();
    return ("");
}