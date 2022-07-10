#pragma once

#include <string>
#include "URI.hpp"
#include "Utils.hpp"
#include "Time.hpp"

namespace HTTP {

class Cookie {

private:
    std::string _domain;
    std::string _path;
    std::string _sameSite;
    std::string _expires;

public:
    std::string name;
    std::string value;

    bool        httpOnly;
    bool        secure;
    
    int32_t     maxAge;

    Cookie(std::string name, std::string value);
    Cookie(Cookie &copy);
    Cookie &operator=(const Cookie &copy);
    ~Cookie(void);

    const std::string   &getDomain(void) const;
    const std::string   &getPath(void) const;
    const std::string   &sameSite(void) const;
    const std::string   &getExpires(void) const;

    void                setDomain(std::string domain);
    void                setPath(std::string path);
    void                setSameSite(std::string sameSite);
    void                setExpires(int day, int month, int year, int hour, int min, int sec);

    const std::string   toString(void) const;
    
};

}