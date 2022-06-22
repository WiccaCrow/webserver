#include "Range.hpp"

namespace HTTP {

bool operator<(const RangeSet &r1, const RangeSet &r2) {

    if ((r1.suffix != -1 || r2.suffix != -1)) {
        return r1.suffix < r2.suffix;
    } else if (r1.beg < r2.beg) {
        return true;
    } else if (r1.beg == r2.beg) {
        return r1.end < r2.end;
    }

    return false;
}

bool operator>(const RangeSet &r1, const RangeSet &r2) {

    if ((r1.suffix != -1 || r2.suffix != -1)) {
        return r1.suffix > r2.suffix;
    } else if (r1.beg > r2.beg) {
        return true;
    } else if (r1.beg == r2.beg) {
        return r1.end > r2.end;
    }

    return false;
}

bool operator==(const RangeSet &r1, const RangeSet &r2) {
    return r1.suffix == r2.suffix && r1.beg == r2.beg && r1.end == r2.end;
}

bool operator>=(const RangeSet &r1, const RangeSet &r2) {
    return !operator<(r1, r2);
}

bool operator<=(const RangeSet &r1, const RangeSet &r2) {
    return !operator>(r1, r2);
}

bool operator!=(const RangeSet &r1, const RangeSet &r2) {
    return !operator==(r1, r2);
}

RangeSet::RangeSet(void) : beg(-1), end(-1), suffix(-1) {}
RangeSet::~RangeSet(void) {}

const std::string
HTTP::RangeSet::to_string(void) {
    std::stringstream ss; 
    ss << "[" << this->beg << ";" << this->end << ";" << this->suffix << "]";
    return ss.str();
}

bool
HTTP::RangeSet::combine(const RangeSet &r2) {
    if (this->suffix != -1 || r2.suffix != -1) {
        return false;
    }

    if (this->beg <= r2.beg && this->end >= r2.beg) {
        this->beg = this->beg < r2.beg ? this->beg : r2.beg; 
        this->end = this->end > r2.end ? this->end : r2.end;
        return true;
    }

    return false;
}

bool
HTTP::RangeSet::parse(const std::string &s) {

    size_t pos = s.find('-');
    if (pos == std::string::npos) {
        return false;
    }

    if (pos == 0) {
        if (!stoll(this->suffix, s.c_str() + 1)) {
            return false;
        } else if (this->suffix < 0) {
            return false;
        }
    } else {

        if (!stoll(this->beg, s.substr(0, pos).c_str())) {
            return false;
        }

        if (s.length() == pos + 1) {
            this->end = LLONG_MAX;
            return true;
        }

        if (!stoll(this->end, s.substr(pos + 1).c_str())) {
            return false;
        }

        if (this->beg > this->end) {
            return false;
        }
    }
    return true;
}


RangeList::RangeList(void) {}
RangeList::~RangeList(void) {}

RangeList::iterator RangeList::begin() {
    return _ranges.begin();
}

RangeList::iterator RangeList::end() {
    return _ranges.end();
}

RangeList::const_iterator RangeList::begin() const {
    return _ranges.begin();
}

RangeList::const_iterator RangeList::end() const {
    return _ranges.end();
}

RangeList::reverse_iterator RangeList::rbegin() {
    return _ranges.rbegin();
}

RangeList::reverse_iterator RangeList::rend() {
    return _ranges.rend();
}

RangeList::const_reverse_iterator RangeList::rbegin() const {
    return _ranges.rbegin();
}

RangeList::const_reverse_iterator RangeList::rend() const {
    return _ranges.rend();
}

RangeList::iterator
RangeList::iter_at(size_t index) {
    iterator it = _ranges.begin();
    std::advance(it, index);
    return it;
}

RangeList::const_iterator
RangeList::iter_at(size_t index) const {
    const_iterator it = _ranges.begin();
    std::advance(it, index);
    return it;
}

bool
RangeList::isValidUnit(void) {
    return _unit == "bytes";
}

bool
RangeList::parse(const std::string &s) {
    _raw = s;

    ltrim(_raw, " ");

    size_t pos = _raw.find('=');
    if (pos == std::string::npos) {
        return false;
    }
    _unit = _raw.substr(0, pos);
    if (!isValidUnit()) {
        return false;
    }
    _raw.erase(0, pos + 1);

    std::vector<std::string> ranges = split(_raw, " ,\r\n");
    if (ranges.empty()) {
        return false;
    }

    for (size_t i = 0; i < ranges.size(); i++) {
        RangeSet set;
        if (!set.parse(ranges[i])) {
            _ranges.clear();
            return false;
        }
        _ranges.push_back(set);
    }
  
    return true;
}

void
RangeList::compress(void) {

    if (!_ranges.size()) {
        return ;
    }

    std::sort(_ranges.begin(), _ranges.end());

    for (size_t i = 0; i < _ranges.size() - 1; ) {        
        // std::cout << _ranges[i].to_string() << " + "
        //           << _ranges[i + 1].to_string() << " = ";
        if (_ranges[i].combine(_ranges[i + 1])) {
            _ranges.erase(iter_at(i + 1));
            i = 0;
        } else {
            i++;
        }
        // std::cout << _ranges[i].to_string() << std::endl;
    }
}

};
