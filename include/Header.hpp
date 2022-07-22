#pragma once

#include <algorithm>
#include <stdint.h>

#include "CRC.hpp"
#include "Utils.hpp"
#include "Status.hpp"
#include "Logger.hpp"
#include "Globals.hpp"
#include "HeaderNames.hpp"
#include "HeadersCodes.hpp"
#include "HTML.hpp"

namespace HTTP {

class Header {
    
    public:
    uint32_t    hash;
    std::string key;
    std::string value;

    Header(void);
    virtual ~Header(void);
    Header(uint32_t hash);
    Header(uint32_t hash, const std::string &value);

    virtual bool isValid(void) = 0;

    bool parse(const std::string &line, bool trimKey = false);

    std::string toString(void);

    friend bool operator==(const Header &h1, const Header &h2);
    friend bool operator!=(const Header &h1, const Header &h2);
};

template<typename T>
class Headers {
public:
    typedef typename std::multimap<uint32_t, T>::key_type key_type;  

    typedef typename std::multimap<key_type, T>::pointer pointer;
    typedef typename std::multimap<key_type, T>::const_pointer const_pointer;
    typedef typename std::multimap<key_type, T>::reference reference;
    typedef typename std::multimap<key_type, T>::const_reference const_reference;

    typedef typename std::multimap<key_type, T>::iterator iterator;
    typedef typename std::multimap<key_type, T>::const_iterator const_iterator;
    typedef typename std::multimap<key_type, T>::reverse_iterator reverse_iterator;
    typedef typename std::multimap<key_type, T>::const_reverse_iterator const_reverse_iterator;

private:
    std::multimap<key_type, T> _c;
    
public:
    iterator begin(void) {
        return _c.begin();
    }

    iterator end(void) {
        return _c.end();
    }

    const_iterator cbegin(void) const {
        return _c.begin();
    }

    const_iterator cend(void) const {
        return _c.end();
    }

    reverse_iterator rbegin(void) {
        return _c.rbegin();
    }

    reverse_iterator rend(void) {
        return _c.rend();
    }

    const_reverse_iterator crbegin(void) const {
        return _c.rbegin();
    }

    const_reverse_iterator crend(void) const {
        return _c.rend();
    }


    bool has(HeaderCode key) {
        return this->has(static_cast<key_type>(key));
    }

    bool has(key_type key) {
        return (_c.find(key) != _c.end());
    }

    std::string &value(key_type key) {
        return this->operator[](key).value;
    }

    std::string &value(HeaderCode key) {
        return this->operator[](static_cast<key_type>(key)).value;
    }

    T &operator[](key_type key) {
        iterator it = _c.find(key);
        if (it == _c.end()) {
            iterator ins = _c.insert(std::make_pair(key, T()));
            ins->second.hash = key;
            return ins->second;
        }
        return it->second;
    }

    T &operator[](HeaderCode key) {
        return this->operator[](static_cast<key_type>(key));
    }

    void insert(T val) {
        _c.insert(std::make_pair<key_type, T>(val.hash, val));
    }

    void erase(key_type key) {
        _c.erase(key);
    }
    
    void erase(HeaderCode key) {
        this->erase(static_cast<key_type>(key));
    }

    void clear(void) {
        _c.clear();
    }
};

} // namespace HTTP
