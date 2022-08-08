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

    int64_t beg;
    int64_t end;
    int64_t suffix;

    RangeSet(void);
    RangeSet(int64_t suffix);
    RangeSet(int64_t beg, int64_t end);
    ~RangeSet(void);

    int64_t size(void);

    bool parse(const std::string &);
    bool combine(const RangeSet &);

    void narrow(int64_t size);
    void rlimit(int64_t limit);

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

    RangeList(void);
    ~RangeList(void);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    RangeSet &operator[](std::size_t index);
    bool empty(void);
    std::size_t size(void);
    
    iterator iter_at(std::size_t);
    const_iterator iter_at(std::size_t) const;

    bool parse(const std::string &);
    void compress(void);
    bool isValidUnit(void);

};

}
