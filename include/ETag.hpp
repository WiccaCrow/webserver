#pragma once

#include <map>
#include <string>

#include "SHA1.hpp"
#include "Time.hpp"
#include "Base64.hpp"

namespace HTTP {

class ETag {

private:
    static std::map<std::string, ETag *> _etags;
    static pthread_mutex_t _lock;

    std::string _tag;
    std::string _mtime_s;
    time_t  _mtime;
    time_t  _atime;

public:
    static void StaticConstructor(void);
    static void StaticDestructor(void);

    ETag(void);
    ~ETag(void);

    void setTag(time_t);
    const std::string &getTag(void);

    time_t getAccessTime(void) const;
    time_t getEntityTime(void) const;
    const std::string &getEntityStrTime(void) const;

    static ETag *get(const std::string &, bool create = true);
};

}