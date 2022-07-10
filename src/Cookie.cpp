#include "Cookie.hpp"

namespace HTTP {

Cookie::Cookie(std::string name, std::string value) : name(name), value(value) {}

Cookie::Cookie(Cookie &copy) {
    *this = copy;
}

Cookie &Cookie::operator=(const Cookie &copy) {
	if (this != &copy) {
        name = copy.name;
        value = copy.value;
        maxAge = copy.maxAge;
        httpOnly = copy.httpOnly;
        secure = copy.secure;
        _domain = copy._domain;
        _path = copy._path;
        _sameSite = copy._sameSite;
        _expires = copy._expires;
	}
	return *this;
}

Cookie::~Cookie(void) {}

const std::string &
Cookie::getDomain() const {
    return _domain;
}

const std::string &
Cookie::getPath() const {
    return _path;
}

const std::string &
Cookie::sameSite() const {
    return _sameSite;
}

const std::string &
Cookie::getExpires() const {
    return _expires;
}

void
Cookie::setDomain(std::string domain) {
    URI uri;
    uri.parse(domain);
    _domain = uri._host;
}

void
Cookie::setPath(std::string path) {
    URI uri;
    uri.parse(path);
    _path = uri._path;
}

void
Cookie::setSameSite(std::string sameSite) {
    toLowerCase(sameSite);
    if (sameSite == "none" || sameSite == "lax" || sameSite == "strict") {
        _sameSite = sameSite;
    }
}

void
Cookie::setExpires(int day, int month, int year, int hour, int min, int sec) {
    struct tm *t = Time::gmtime();
    t->tm_mday = day;
    t->tm_mon = month - 1;
    t->tm_year = year;
    t->tm_hour = hour;
    t->tm_min = min;
    t->tm_sec = sec;
    _expires = Time::time2str(t, Time::f_gmt);
}


const std::string
Cookie::toString() const {
    std::string str = name + "" + value + ";";

    if (!_domain.empty()) {
        str += " Domain=" + _domain + ";";
    }
    if (!_path.empty()) {
        str += " Path=" + _path + ";";
    }
    if (!_expires.empty()) {
        str += " Expires=" + _expires + ";";
    }
    if (maxAge != 0) {
        str += " MaxAge=" + NumberToString(maxAge) + ";";
    }
    if (!_sameSite.empty()) {
        str += " SameSite=" + _sameSite + ";";
    }
    if (secure) {
        str += " Secure;";
    }
    if (httpOnly) {
        str += " HttpOnly;";
    }

    return str;
}

}

// int main() {
//     HTTP::Cookie c("id", "1");

//     std::cout << c.toString() << std::endl;
// }