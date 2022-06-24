#include "Auth.hpp"

namespace HTTP {

Auth::Auth(void) : _set(false) {}

Auth::~Auth(void) {}

static bool 
splitCredentials(const std::string &line, std::pair<std::string, std::string> &ref) {
    
    std::pair<std::string, std::string> crds;
    size_t pos = line.find(':');
    if (pos == std::string::npos) {
        return false;
    }
    crds.first = line.substr(0, pos);
    crds.second = line.substr(pos + 1);
    ref = crds;
    return true;
}

bool 
Auth::loadData(void) {
    std::ifstream in;

    in.open(_file.c_str());

    if (!in.is_open() || !in.good()) {
        Log.error() << "Auth::Cannot open file " << _file << Log.endl;
        return false;
    }

    std::string line;
    for ( ;std::getline(in, line); ) {
        std::pair<std::string, std::string> crds;
        if (!splitCredentials(line, crds)) {
            Log.error() << "Auth::Data file corrupted: " << line << Log.endl;
            return false;
        }
        _data.insert(crds);
    }
    return true;
}

std::string &
Auth::getFileRef(void) {
    return _file;
}

std::string &
Auth::getRealmRef(void) {
    return _realm;
}

bool
Auth::isSet(void) const {
    return _set;
}

void
Auth::set(bool set) {
    _set = set;
}

void
Auth::setFile(const std::string &file) {
    _file = file;
}

void 
Auth::setRealm(const std::string &realm) {
    _realm = realm;
}

bool 
Auth::isAuthorized(const std::string line) const {
    char salt[2] = { 0 };
    std::vector<std::string> crds = split(line, ":");
    if (crds.size() < 2) {
        Log.error() << "Auth::invalid credentials of the user: " << line << Log.endl;
        return false;
    }

    std::map<std::string, std::string>::const_iterator it = _data.find(crds[0]);
    if (it == _data.end()) {
        Log.debug() << "Auth::user not found: " << crds[0] << Log.endl;
        return false;
    }

    salt[0] = it->second[0];
    salt[1] = it->second[1];
    std::string encrypted = crypt(crds[1].c_str(), salt);
    if (it->second != encrypted) {
        Log.debug() << "Auth::passwords do not match" << Log.endl;
        Log.debug() << "stored: " + it->second << Log.endl;
        Log.debug() << "encrypted: " + encrypted << Log.endl;
        return false;
    }
    return true;
}

}