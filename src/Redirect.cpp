#include "Redirect.hpp"

namespace HTTP {

Redirect::Redirect(void)
    : _code(MOVED_PERMANENTLY)
    , _set(false) { }

Redirect::~Redirect(void) { }

StatusCode &
Redirect::getCodeRef(void) {
    return _code;
}

std::string &
Redirect::getURIRef(void) {
    return _uri;
}

void
Redirect::set(bool val) {
    _set = val;
}

bool
Redirect::set(void) {
    return _set;
}

}
