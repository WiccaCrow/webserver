#include "Redirect.hpp"

Redirect::Redirect(void) : _code(0), _uri(""), _set(false) {}
Redirect::~Redirect(void) {}

int &Redirect::getCodeRef(void) {
    return _code;
}

std::string &Redirect::getURIRef(void) {
    return _uri;
}

bool Redirect::toggle(void) {
    _set = !_set;
    return _set;
}

bool Redirect::isSet(void) {
    return _set;
}
