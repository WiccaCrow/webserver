#include "Utils.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <fstream>

// Returns true if the conversion of a string to a number is true
// and initialize num argument
bool
stoll(long long &num, char const *s) {
    char              c;
    std::stringstream ss(s);
    ss >> num;
    return !(ss.fail() || ss.get(c));
}

bool
stoi64(int64_t &num, const std::string &s) {
    char              c;
    std::stringstream ss(s.c_str());
    ss >> num;
    return !(ss.fail() || ss.get(c));
}

// Returns true if the conversion of a string to a number is true
bool
stoll(char const *s) {
    long long         num;
    char              c;
    std::stringstream ss(s);
    ss >> num;
    return !(ss.fail() || ss.get(c));
}

std::vector<std::string>
split(const std::string &source, const std::string &delimiters) {
    std::size_t prev = 0;
    std::size_t curr = 0;

    std::vector<std::string> results;
    while ((curr = source.find_first_of(delimiters, prev)) != std::string::npos) {
        if (curr > prev) {
            results.push_back(source.substr(prev, curr - prev));
        }
        prev = curr + 1;
    }

    if (prev < source.length()) {
        results.push_back(source.substr(prev));
    }

    return results;
}

std::string
join(std::vector<std::string> &v, const std::string &delim) {
    std::string res;
    for (std::size_t i = 0; i < v.size(); ++i) {
        res += v[i];
        if (i + 1 < v.size()) {
            res += delim;
        }
    }
    return res;
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
    std::size_t length = s.length();
    for (std::size_t i = 0; i < length; ++i) {
        s[i] = std::tolower(s[i]);
    }
}

void
writeFile(const std::string &file, const std::string &content) {
    std::ofstream out(file.c_str(),
        std::ios_base::out | std::ios_base::trunc);
    if (out.is_open() && out.good()) {
        out << content;
    }
}

std::string
readFile(const std::string &filename) {
    std::ifstream in(filename.c_str());
    if (!in.is_open()) {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::string
getWord(const std::string &line, const char *delims, std::size_t &pos) {
    std::size_t tmp = pos;
    std::size_t end = pos = line.find(delims, pos);

    if (end == std::string::npos)
        end = pos = line.length();
    return line.substr(tmp, end - tmp);
}

void
skipSpaces(const std::string &line, std::size_t &pos) {
    for (; line[pos] == ' '; pos++)
        ;
}

#if __cplusplus < 201103L

std::string
ultos(unsigned long val) {
    char buf[25];
    snprintf(buf, 25, "%lu", val);
    return std::string(buf);
}

std::string
ulltos(unsigned long long val) {
    char buf[25];
    snprintf(buf, 25, "%llu", val);
    return std::string(buf);
}

std::string
lltos(long long val) {
    char buf[25];
    snprintf(buf, 25, "%lld", val);
    return std::string(buf);
}

std::string
ltos(long val) {
    char buf[25];
    snprintf(buf, 25, "%ld", val);
    return std::string(buf);
}

std::string
itos(int val) {
    char buf[25];
    snprintf(buf, 25, "%d", val);
    return std::string(buf);
}

std::string
sztos(std::size_t val) {
    char buf[25];
    snprintf(buf, 25, "%zu", val);
    return std::string(buf);
}

#endif


std::size_t strlen_u8(const std::string &s) {

    std::size_t ic = 0; //char index

    for (std::size_t i = 0; i < s.length(); i++, ic++) {
        int c = static_cast<unsigned char>(s[i]);
        if (!isascii(c)) {
            if ((c & 0xE0) == 0xC0) {
                i += 1;
            } else if ((c & 0xF0) == 0xE0) {
                i += 2;
            } else if ((c & 0xF8) == 0xF0) {
                i += 3;
            } else {
                return 0;
            }
        }
    }
    return ic;
}

// RFC validation part, should be moved to another file, maybe URI.cpp

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

const char * validMethods[] = {
    "GET", "DELETE", "POST",
    "PUT", "HEAD", "CONNECT",
    "OPTIONS", "TRACE", "PATCH", 
    NULL
};

bool
isValidMethod(const std::string &method) {

    for (int i = 0; validMethods[i]; ++i) {
        if (validMethods[i] == method)
            return true;
    }
    return false;
}

bool
isValidProtocol(const std::string &protocol) {
    if (protocol.find("HTTP/") != 0) {
        return false;
    }
    if (!isdigit(protocol[5]) || protocol[6] != '.') {
        return false;
    }
    if (!isdigit(protocol[7]) || protocol.length() != 8) {
        return false;
    }
    return true;
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
isValidScheme(const std::string &s) {
    for (std::size_t i = 0; i < s.size(); i++) {
        if (!isalnum(s[i]) && !strchr("+-.", s[i])) {
            return false;
        }
    }
    return true;
}

bool
isValidPort(const std::string &s) {
    for (std::size_t i = 0; i < s.size(); i++) {
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

bool
isValidUserInfo(const std::string &s) {
    for (std::size_t i = 0; i < s.length(); ++i) {
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
    std::size_t posA = s.find("@");
    if (posA != std::string::npos) {
        if (!isValidUserInfo(s.substr(0, posA))) {
            return false;
        }
    } else {
        posA = 0;
    }
    std::size_t posC = s.find(":", posA + 1);
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

std::size_t
isSegmentNz(const std::string &s) {

    for (std::size_t i = 0; i < s.length(); ++i) {
        if (isPctEncoded(s.c_str() + i)) {
            i += 2;
        } else if (!isUnreserved(s[i]) && !isSubDelims(s[i]) && !strchr("@:", s[i]) ) {
            return i;
        }
    }
    return s.length();
}

std::size_t
isSegmentNzNc(const std::string &s) {

    for (std::size_t i = 0; i < s.length(); ++i) {
        if (isPctEncoded(s.c_str() + i)) {
            i += 2;
        } else if (!isUnreserved(s[i]) && !isSubDelims(s[i]) && s[i] != '@') {
            return i;
        }
    }
    return s.length();
}

std::size_t
isValidSegment(const std::string &s) {

    if (s[0] != '/') {
        return 0;
    }

    for (std::size_t i = 1; i < s.length(); ++i) {
        if (isPctEncoded(s.c_str() + i)) {
            i += 2;
        } else if (!isUnreserved(s[i]) && !isSubDelims(s[i]) && !strchr("@:", s[i])) {
            return i;
        }
    }
    return s.length();
}


// segment       = *pchar
// segment-nz    = 1*pchar
// segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//                   non-zero-length segment without any colon ":"

// pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

bool
isValidPathAbempty(const std::string &s) {
    // *( "/" segment )

    for (std::size_t i = 0; i < s.size(); ) {
        std::size_t pos = isValidSegment(s.substr(i));
        if (pos == 0) {
            return false;
        }
        else if (pos == s.length()) {
            return true;
        }
        i += pos;
    }
    return true;
}

bool
isValidPathAbsolute(const std::string &s) {
    // "/" [ segment-nz *( "/" segment ) ]

    if (s[0] != '/') {
        return false;
    } else if (s.length() == 1) {
        return true;
    } else {
        std::size_t pos = isSegmentNz(s.substr(1));
        if (pos == 0) {
            return false;
        } else if (pos == s.length()) {
            return true;
        } else {
            return isValidPathAbempty(s.substr(pos + 1));
        }
    }
}


bool
isValidPathNoScheme(const std::string &s) {
    // segment-nz-nc *( "/" segment )  

    std::size_t pos = isSegmentNzNc(s);
    if (pos == 0) {
        return false;
    } else if (pos == s.length()) {
        return false;
    } else {
        return isValidPathAbempty(s.substr(pos));
    }
}

bool
isValidPathRootless(const std::string &s) {
    // segment-nz *( "/" segment )

    std::size_t pos = isSegmentNz(s);
    if (pos == 0) {
        return false;
    } else if (pos == s.length()) {
        return false;
    } else {
        return isValidPathAbempty(s.substr(pos));
    }
}

bool
isValidPathEmpty(const std::string &s) {
    // 0<pchar>

    return s.empty();
}

bool
isValidRegName(const std::string &regname) {
    if (regname.length() > 255) {
        return false;
    }
    for (std::size_t i = 0; i < regname.length(); ++i) {
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
    return isValidPathAbempty(path) ||
           isValidPathAbsolute(path) || 
           isValidPathRootless(path) ||
           isValidPathNoScheme(path) || 
           isValidPathEmpty(path);
}

bool
isWord(const std::string word) {
    for (std::size_t i = 0; i < word.length(); i++) {
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
    return (str.find(end, str.length() - end.length()) != std::string::npos);
}

bool
startsWith(const std::string &str, const std::string &beg) {
    return (str.find(beg) == 0);
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

time_t
getModifiedTime(const std::string &file) {
    
    struct stat st;
    if (stat(file.c_str(), &st) < 0) {
        return 0;
    }

    return st.st_mtime;
}


