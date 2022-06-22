#pragma once

#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <limits.h>

#include "Utils.hpp"

namespace HTTP {

struct RangeSet {

    long long beg;
    long long end;
    long long suffix;

    RangeSet(void);
    ~RangeSet(void);

    bool parse(const std::string &);
    bool combine(const RangeSet &);

    const std::string to_string(void);

    friend bool operator<(const RangeSet &r1, const RangeSet &r2);
    friend bool operator>(const RangeSet &r1, const RangeSet &r2);
    friend bool operator==(const RangeSet &r1, const RangeSet &r2);
    friend bool operator!=(const RangeSet &r1, const RangeSet &r2);
    friend bool operator<=(const RangeSet &r1, const RangeSet &r2);
    friend bool operator>=(const RangeSet &r1, const RangeSet &r2);
};

class RangeList {

public:
    typedef std::deque<RangeSet>::iterator iterator;
    typedef std::deque<RangeSet>::const_iterator const_iterator;
    typedef std::deque<RangeSet>::reverse_iterator reverse_iterator;
    typedef std::deque<RangeSet>::const_reverse_iterator const_reverse_iterator;

private:
    std::string          _raw;
    std::string          _unit;
    std::deque<RangeSet> _ranges;

public:
    // RangeList(const std::string &);

    RangeList(void);
    ~RangeList(void);

    // void push_back();
    // void pop_back();
    // void remove();
    // void insert();

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    // operator[];
    
    iterator iter_at(size_t);
    const_iterator iter_at(size_t) const;

    bool parse(const std::string &);
    void compress(void);
    bool isValidUnit(void);

};

}
