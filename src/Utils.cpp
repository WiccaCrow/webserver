#include "Utils.hpp"
#include "SHA1.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <vector>

std::vector<std::string>
split(const std::string &source, const std::string &delimiters = " ") {
    size_t                   prev       = 0;
    size_t                   currentPos = 0;
    std::vector<std::string> results;

    while ((currentPos = source.find_first_of(delimiters, prev)) != std::string::npos) {
        if (currentPos > prev) {
            results.push_back(source.substr(prev, currentPos - prev));
        }
        prev = currentPos + 1;
    }

    if (prev < source.length()) {
        results.push_back(source.substr(prev));
    }

    return results;
}

std::string
itoh(int nb) {
    std::string inhex;
    for (int hexDig = nb % 16; nb; hexDig = nb % 16) {
        if (hexDig > 9) {
            hexDig += 'A' - 10;
        } else {
            hexDig += '0';
        }
        inhex = (char)hexDig + inhex;
        nb /= 16;
    }
    return inhex;
}

inline std::string &
rtrim(std::string &s, const char *t) {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

inline std::string &
ltrim(std::string &s, const char *t) {
    s.erase(0, s.find_first_not_of(t));
    return s;
}

void
trim(std::string &s, const char *t) {
    ltrim(rtrim(s, t), t);
}

void
toLowerCase(std::string &s) {
    size_t length = s.length();
    for (size_t i = 0; i < length; ++i) {
        s[i] = tolower(s[i]);
    }
}

std::string
getWord(const std::string &line, const char *delims, size_t &pos) {
    size_t tmp = pos;
    size_t end = pos = line.find(delims, pos);

    if (end == std::string::npos)
        end = pos = line.length();
    return line.substr(tmp, end - tmp);
}

void
skipSpaces(const std::string &line, size_t &pos) {
    for (; line[pos] == ' '; pos++)
        ;
}

#if __cplusplus < 201103L

std::string
to_string(unsigned long val) {
    char buf[25];
    snprintf(buf, 25, "%lu", val);
    return std::string(buf);
}

#endif

bool
isUnreserved(int c) {
    return (std::isalnum(c) || std::strchr("-._~", c) != NULL);
}

bool
isSubDelims(int c) {
    return (std::strchr("!$&'()*+,;=", c) != NULL);
}

bool
isGenDelims(int c) {
    return (std::strchr(":/?#[]@", c) != NULL);
}

bool
isPctEncoded(const char *s) {
    return (s[0] == '%' && std::isxdigit(s[1]) && std::isxdigit(s[2]));
}

bool
isPchar(const char *s) {
    return isUnreserved(*s) || isPctEncoded(s) || isSubDelims(*s) || !strchr("@:", *s);
}

bool
isValidOctet(char *octet) {
    if (octet[0] == '0' && octet[1] != '\0')
        return false;

    for (int i = 0; octet[i] != '\0'; i++) {
        if (!isdigit(octet[i]))
            return false;
    }

    int x = atoi(octet);
    return (x >= 0 && x <= 255);
}

bool
isValidIp(const std::string &ip) {
    char buf[32];
    strncpy(buf, ip.c_str(), 32);

    int   count = 0;
    char *octet = strtok(buf, ".");
    while (octet) {
        count++;
        if (!isValidOctet(octet)) {
            return 0;
        }
        octet = strtok(NULL, ".");
    }
    return (count == 4);
}

bool
isValidScheme(const std::string &s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (!isalnum(s[i]) && !strchr("+-.", s[i])) {
            return false;
        }
    }
    return true;
}

bool
isValidPort(const std::string &s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

bool
isValidUserInfo(const std::string &s) {
    for (size_t i = 0; i < s.length(); ++i) {
        if (isPctEncoded(s.c_str() + i)) {
            i += 2;
        } else if (!isUnreserved(s[i]) && !isSubDelims(s[i]) && s[i] != ':') {
            return false;
        }
    }
    return true;
}


bool
isValidAuthority(const std::string &s) {
    size_t posA = s.find("@");
    if (posA != std::string::npos) {
        if (!isValidUserInfo(s.substr(0, posA))) {
            return false;
        }
    } else {
        posA = 0;
    }
    size_t posC = s.find(":", posA + 1);
    if (posC != std::string::npos) {
        if (!isValidPort(s.substr(posC + 1))) {
            return false;
        }
    }
    if (!isValidHost(s.substr(posA + 1, posC - posA))) {
        return false;
    }
    return true;
}

bool isValidIpv6(const std::string &s) {
    struct in6_addr addr;
    return inet_pton(PF_INET6, s.c_str(), &addr);
}

bool isValidIpv4(const std::string &s) {
    struct in_addr addr;
    return inet_pton(PF_INET, s.c_str(), &addr);
}

bool
isValidIpvFuture(const std::string &s) {
    return s[0] == 'v' && isxdigit(s[1]) && s[2] != '.' && 
        (s[3] != ':' || isUnreserved(s[3]) || !isSubDelims(s[3]));
}

bool
isValidIpLiteral(const std::string &s) {
    return isValidIpvFuture(s) || isValidIpv6(s);
}

// bool
// isSegmentNz() {
// }

// bool
// isSegmentNzNc() {
// }

bool
isValidRegName(const std::string &regname) {
    if (regname.length() > 255) {
        return false;
    }
    for (size_t i = 0; i < regname.length(); ++i) {
        if (isPctEncoded(regname.c_str() + i)) {
            i += 2;
        } else if (!isUnreserved(regname[i]) && !isSubDelims(regname[i])) {
            return false;
        }
    }
    return true;
}

bool
isValidHost(const std::string &s) {
    return (isValidIpLiteral(s) || isValidIpv4(s) || isValidRegName(s));
}

bool
isValidPath(const std::string &path) {

    if (path[0] != '/') {
        return false;
    } else if (path.find("//") != std::string::npos) {
        return false;
    }

    char dst[512];
    strcpy(dst, path.c_str());

    char *token = strtok(dst, "/");
    while (token != NULL) {
        size_t len = strlen(token);
        for (size_t i = 1; i < len; i++) {
            if (!isalnum(token[i])) {
                return false;
            }
        }
        token = strtok(NULL, "/");
    }
    return true;
}

bool
isWord(const std::string word) {
    for (size_t i = 0; i < word.length(); i++) {
        if (!isalnum(word[i]))
            return false;
    }
    return true;
}

bool
isExtension(const std::string &ext) {
    if (ext[0] != '.')
        return false;
    return (isWord(&ext[1]));
}

bool
endsWith(const std::string &str, const std::string &end) {
    if (end.length() >= str.length())
        return false;
    return str.find(end, str.length() - end.length()) != std::string::npos;
}

bool
resourceExists(const std::string &filename) {
    struct stat state;

    return (stat(filename.c_str(), &state) == 0);
}

bool
isFile(const std::string &filename) {
    struct stat state;

    if (stat(filename.c_str(), &state) < 0)
        return false;
    return S_ISREG(state.st_mode);
}

bool
isDirectory(const std::string &dirname) {
    struct stat state;

    if (stat(dirname.c_str(), &state) < 0)
        return false;
    return S_ISDIR(state.st_mode);
}

bool
checkRegFilePerms(const std::string &filename, int perm) {
    struct stat state;

    if (stat(filename.c_str(), &state) < 0)
        return false;
    if (!S_ISREG(state.st_mode))
        return false;
    if ((state.st_mode & perm) != 0)
        return true;
    return false;
}

bool
isReadableFile(const std::string &filename) {
    return checkRegFilePerms(filename, S_IREAD); // User rights only
}

bool
isWritableFile(const std::string &filename) {
    return checkRegFilePerms(filename, S_IWRITE); // User rights only
}

bool
isExecutableFile(const std::string &filename) {
    return checkRegFilePerms(filename, S_IEXEC); // User rights only
}

int
rmdirNonEmpty(std::string &resourceDel) {
    struct dirent *dirContent;
    std::string    path;

    DIR *r_opndir;
    if (!(r_opndir = opendir(resourceDel.c_str()))) {
        return 1;
    }

    while ((dirContent = readdir(r_opndir))) {
        if ((static_cast<std::string>(dirContent->d_name) != ".") && (static_cast<std::string>(dirContent->d_name) != "..")) {
            path = resourceDel + '/' + dirContent->d_name;

            if (isDirectory(path)) {
                if (rmdirNonEmpty(path)) {
                    closedir(r_opndir);
                    return 1;
                }
            } else if (remove(path.c_str())) {
                closedir(r_opndir);
                return 1;
            }
        }
    }

    closedir(r_opndir);
    if (remove(resourceDel.c_str())) {
        return 1;
    }

    return 0;
}

static std::string
getTimeStringGMT(time_t *time) {
    struct tm *info = gmtime(time);

    char buff[50];
    strftime(buff, sizeof(buff), "%a, %-e %b %Y %H:%M:%S GMT", info);
    return buff;
}

std::string
getDateTimeGMT() {

    time_t rawtime;
    time(&rawtime);

    return getTimeStringGMT(&rawtime);
}

std::string
getEtagFile(const std::string &filename) {
    struct stat state;

    if (stat(filename.c_str(), &state) < 0) {
        return "";
    }
    return SHA1(to_string(state.st_mtime));
}

std::string
getLastModifiedTimeGMT(const std::string &filename) {
    struct stat state;

    if (stat(filename.c_str(), &state) < 0) {
        return "";
    }

    return getTimeStringGMT(&state.st_mtime);
}
