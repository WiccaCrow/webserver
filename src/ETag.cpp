#include "ETag.hpp"

namespace HTTP {

std::map<std::string, ETag *> ETag::_etags;
pthread_mutex_t ETag::_lock;

void
ETag::StaticConstructor(void) {
    pthread_mutex_init(&_lock, NULL);
}

void
ETag::StaticDestructor(void) {
    std::map<std::string, ETag *>::iterator it;
    for (it = _etags.begin(); it != _etags.end(); ++it) {
        if (it->second != NULL) {
            delete it->second;
        }
    }

    pthread_mutex_destroy(&_lock);
}


ETag::ETag(void) {
    _atime = Time::now();
    _mtime = 0;
}

ETag::~ETag(void) {}

void
ETag::setTag(time_t time) {

    pthread_mutex_lock(&ETag::_lock);

    _atime = Time::now();
    _mtime = time;
    _mtime_s = Time::gmt(time);
    _tag = Base64::encode(SHA1().hash(_mtime_s));

    pthread_mutex_unlock(&ETag::_lock);
}

const std::string &
ETag::getTag(void) {
    _atime = Time::now();
    return _tag;
}

time_t
ETag::getAccessTime(void) const {
    return _atime;
}

time_t
ETag::getEntityTime(void) const {
    return _mtime;
}

const std::string &
ETag::getEntityStrTime(void) const {
    return _mtime_s;
}

ETag *ETag::get(const std::string &key, bool create) {

    ETag *tag = NULL;

    pthread_mutex_lock(&ETag::_lock);

    std::map<std::string, ETag *>::iterator it = _etags.find(key);
    if (it != _etags.end()) {
        tag = it->second; 
    } else if (create) {
        tag = new ETag();
        _etags.insert(std::make_pair(key, tag));
    }

    pthread_mutex_unlock(&ETag::_lock);

    return tag;
}


}