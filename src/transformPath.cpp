#include <cstring>
#include <string>
#include <vector>

std::string transformPath(const std::string &path, std::vector<Location> &locations) {
    int    locationIndex = -1;
    size_t locationMax = 0;

    // find location
    const size_t size = locations.size();
    for (size_t i = 0; i < size; i++) {
        size_t len = locations[i].path.length();
        size_t pos = strncmp(path.c_str(), locations[i].path.c_str(), len);
        if (pos == 0) {
            if (len > locationMax) {
                locationMax = len;
                locationIndex = i;
            }
        }
    }
    if (locationIndex != -1) {
        // To much allocating, need to improve
        // Also need to decide the rules of putting / in root or location
        return locations[locationIndex].root + path.substr(locationMax + 1);
    } else {
        // Try to check default root or leave it
        return path;
    }
}

//#include <iostream>

// struct Location {
//     std::string path;
//     std::string root;
// };
//
// int main() {
//     std::vector<Location> locs;
//     locs.push_back((Location){"/ponys/red", "/var/red/"});
//     locs.push_back((Location){"/ponys/white", "/var/white/"});
//     locs.push_back((Location){"/ponys", "/var/ponys/"});
//     locs.push_back((Location){"/", "/root/"});
//     //locs.push_back((Location){"", "/var/www/"});
//     //locs.push_back((Location){"", "/var/www/"});
//
//     std::cout << transformPath("/ponys/red/pony.py", locs) << std::endl;
// }