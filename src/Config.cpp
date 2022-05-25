#include "Config.hpp"

int isInteger(double &num) {
    return (num - static_cast<long long>(num) == 0);
}

int isUInteger(double &num) {
    return (isInteger(num) && num >= 0);
}

std::string getDataTypeName(ExpectedType type) {
    switch (type) {
        case ARRAY:
            return "array";
        case NUMBER:
            return "number";
        case OBJECT:
            return "object";
        case STRING:
            return "string";
        case BOOLEAN:
            return "boolean";
        default:
            return "unknown";
    }
}

int typeExpected(JSON::AType *ptr, ExpectedType type) {
    switch (type) {
        case STRING:
            return ptr->isStr();
        case BOOLEAN:
            return ptr->isBool();
        case NUMBER:
            return ptr->isNum();
        case OBJECT:
            return ptr->isObj();
        case ARRAY:
            return ptr->isArr();
    }
    return 0;
}

template <typename T>
int basicCheck(JSON::Object *src, const std::string &key, ExpectedType type, T &res, T def) {
    JSON::AType *ptr = src->get(key);
    if (ptr->isNull()) {
        res = def;
        Log.info("Optional parameter \"" + key + "\" is not found (default used).");
        return 2;
    }

    if (!typeExpected(ptr, type)) {
        Log.error("\"" + key + "\": expected " + getDataTypeName(type) + ", got " + ptr->getType());
        return 0;
    }
    return 1;
}

int basicCheck(JSON::Object *src, const std::string &key, ExpectedType type) {
    JSON::AType *ptr = src->get(key);
    if (ptr->isNull()) {
        Log.error("\"" + key + "\" does not exist.");
        return 0;
    }

    if (!typeExpected(ptr, type)) {
        Log.error("\"" + key + "\": expected " + getDataTypeName(type) + ", got " + ptr->getType());
        return 0;
    }
    return 1;
}

int getUInteger(JSON::Object *src, const std::string &key, ExpectedType type, int &res, int def) {
    switch (basicCheck(src, key, type, res, def)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    double num = src->get(key)->toNum();
    if (isUInteger(num)) {
        res = static_cast<unsigned int>(num);
        return 1;
    } else {
        Log.error(key + ": should be an unsigned integer.");
        return 0;
    }
}

int getUInteger(JSON::Object *src, const std::string &key, ExpectedType type, int &res) {
    switch (basicCheck(src, key, type)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    double num = src->get(key)->toNum();
    if (isUInteger(num)) {
        res = static_cast<unsigned int>(num);
        return 1;
    } else {
        Log.error(key + ": should be an unsigned integer.");
        return 0;
    }
}

int getString(JSON::Object *src, const std::string &key, ExpectedType type, std::string &res, std::string def) {
    switch (basicCheck(src, key, type, res, def)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    res = src->get(key)->toStr();
    return 1;
}

int getString(JSON::Object *src, const std::string &key, ExpectedType type, std::string &res) {
    switch (basicCheck(src, key, type)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    res = src->get(key)->toStr();
    return 1;
}

int getBoolean(JSON::Object *src, const std::string &key, ExpectedType type, bool &res, bool def) {
    switch (basicCheck(src, key, type, res, def)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    res = src->get(key)->toBool();
    return 1;
}

int getBoolean(JSON::Object *src, const std::string &key, ExpectedType type, bool &res) {
    switch (basicCheck(src, key, type)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    res = src->get(key)->toBool();
    return 1;
}

template <typename T>
int isSubset(std::vector<T> set, std::vector<T> subset) {
    typename std::vector<T>::iterator it = subset.begin();
    typename std::vector<T>::iterator end = subset.end();
    for (; it != end; it++) {
        if (std::find(set.begin(), set.end(), *it) == set.end()) {
            return 0;
        }
    }
    return 1;
}

int getArray(JSON::Object *src, const std::string &key, ExpectedType type, std::vector<std::string> &res, std::vector<std::string> def) {
    switch (basicCheck(src, key, type, res, def)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    JSON::Array *arr = src->get(key)->toArr();
    
    JSON::Array::iterator it = arr->begin();
    JSON::Array::iterator end = arr->end();
    for (; it != end; it++) {
        if ((*it)->isNull() || !(*it)->isStr()) {
            Log.error(key + " has mixed value(s)");
            return 0;
        }
        res.push_back((*it)->toStr());
    }
    return 1;
}

int getArray(JSON::Object *src, const std::string &key, ExpectedType type, std::vector<std::string> &res) {
    switch (basicCheck(src, key, type)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    JSON::Array *arr = src->get(key)->toArr();

    JSON::Array::iterator it = arr->begin();
    JSON::Array::iterator end = arr->end();
    for (; it != end; it++) {
        if ((*it)->isNull() || !(*it)->isStr()) {
            Log.error(key + " has mixed value(s)");
            return 0;
        }
        res.push_back((*it)->toStr());
    }
    return 1;
}

// Default values
std::vector<std::string> getDefaultAllowedMethods() {
    std::vector<std::string> allowed(9);

    allowed.push_back("GET");
    allowed.push_back("DELETE");
    allowed.push_back("POST");
    allowed.push_back("PUT");
    allowed.push_back("HEAD");
    allowed.push_back("CONNECT");
    allowed.push_back("OPTIONS");
    allowed.push_back("TRACE");
    allowed.push_back("PATCH");

    return allowed;
}

std::vector<std::string> getDefaultIndex() {
    std::vector<std::string> def(0);

    return def;
}

// Object parsing
int parseCGI(JSON::Object *src, std::map<std::string, std::string> &res) {
    switch (basicCheck(src, "CGI", OBJECT, res, res)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }
    
    JSON::Object *obj = src->get("CGI")->toObj();

    JSON::Object::iterator it = obj->begin();
    JSON::Object::iterator end = obj->end();
    for (; it != end; it++) {
        std::string value = it->second->toStr();
        res.insert(std::make_pair(it->first, value));
    }
    return 1;
}

int isValidCGI(std::map<std::string, std::string> &res) {
    std::map<std::string, std::string>::iterator it = res.begin();
    std::map<std::string, std::string>::iterator end = res.end();

    for (; it != end; it++) {
        if (!isExtension(it->first)) {
            Log.error("\"" + it->first + "\": incorrect extension");
            return false;
        }
        if (!isExecutableFile(it->second)) {
            Log.error("\"" + it->second + "\" is not executable file");
            return false;
        }
    }
    return true;
}

int parseErrorPages(JSON::Object *src, std::map<int, std::string> &res) {
    switch (basicCheck(src, "error_pages", OBJECT, res, res)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    JSON::Object *errObj = src->get("error_pages")->toObj();

    JSON::Object::iterator it = errObj->begin();
    JSON::Object::iterator end = errObj->end();
    for (; it != end; it++) {
        double value = strtod(it->first.c_str(), NULL);
        if (!isUInteger(value) || value > 999) {
            return 0;
        }
        int code = static_cast<int>(value);
        if (it->second->isNull() || !it->second->isStr()) {
            return 0;
        }
        res.insert(std::make_pair(code, it->second->toStr()));
    }
    return 1;
}

int isValidErrorPages(std::map<int, std::string> &res) {
    std::map<int, std::string>::iterator it = res.begin();
    std::map<int, std::string>::iterator end = res.end();

    for (; it != end; it++) {
        if (!fileExists(it->second)) {
            Log.error("\"" + it->second + "\": file does not exist");
            return false;
        }
        if (!isReadableFile(it->second)) {
            Log.error("\"" + it->second + "\": is not readable file");
            return false;
        }
    }
    return true;
}

int parseRedirect(JSON::Object *src, Redirect &res) {
    switch (basicCheck(src, "redirect", OBJECT, res, res)) {
        case 1: break;
        case 2: return 1;
        default: return 0;
    }

    JSON::Object *rd = src->get("redirect")->toObj();

    if (!getUInteger(rd, "code", NUMBER, res.getCodeRef()))
        return 0;
    
    if (!getString(rd, "uri", STRING, res.getURIRef()))
        return 0;

    res.toggle();
    return 1;
}

int isValidRedirect(Redirect &res) {

    if (res.isSet()) {
        if (res.getCodeRef() < 300 && res.getCodeRef() > 308) {
            Log.error("Redirect code \"" + to_string(res.getCodeRef()) + "\"is invalid");
            return 0;
        }
        else if (res.getURIRef() == "") {
            Log.error("Redirect uri is empty");
            return 0;
        }
    }
    return 1;
}

int parseLocation(JSON::Object *src, Location &dst, Location &def) {
    if (!getString(src, "root", STRING, dst.getRootRef(), def.getRootRef())) { // optional ?
        Log.error("#### Failed to parse \"root\"");
        return 0;
    } else if (!fileExists(dst.getRootRef())) {
        Log.error("#### \"root\": " + dst.getRootRef() + " does not exist");
        return 0;
    } else if (!isDirectory(dst.getRootRef())) {
        Log.error("#### \"root\" should be a directory");
        return 0;
    }

    if (!getString(src, "default_page", STRING, dst.getDefaultPageRef())) {
        Log.error("#### Failed to parse \"default_page\"");
        return 0;
    } else if (!isReadableFile(dst.getDefaultPageRef())) {
        Log.error("#### \"default_page\" " + dst.getDefaultPageRef() + "is not regular readable file");
        return 0;
    }

    if (!getUInteger(src, "post_max_body", NUMBER, dst.getPostMaxBodyRef(), 200)) {
        Log.error("#### Failed to parse \"post_max_body\"");
        return 0;
    }

    if (!getBoolean(src, "autoindex", BOOLEAN, dst.getAutoindexRef(), false)) {
        Log.error("#### Failed to parse \"autoindex\"");
        return 0;
    }

    if (!parseRedirect(src, dst.getRedirectRef())) {
        Log.error("#### Failed to parse \"redirect\"");
        return 0;
    } else if (!isValidRedirect(dst.getRedirectRef())) {
        return 0;
    }

    if (!parseCGI(src, dst.getCGIPathsRef())) {
        Log.error("#### Failed to parse \"CGI\"");
        return 0;
    } else if (!isValidCGI(dst.getCGIPathsRef())) {
        Log.error("#### Invalid \"CGI\". Prototype: \"extension\": \"path-to-executable\"");
        return 0;
    }

    if (!getArray(src, "methods-allowed", ARRAY, dst.getAllowedMethodsRef(), getDefaultAllowedMethods())) {
        Log.error("#### Failed to parse \"methods-allowed\"");
        return 0;
    } else if (!isSubset(getDefaultAllowedMethods(), dst.getAllowedMethodsRef())) {
        Log.error("#### Unrecognized value in \"methods-allowed\"");
        return 0;
    }

    if (!getArray(src, "index", ARRAY, dst.getIndexRef(), getDefaultIndex())) {
        Log.error("#### Failed to parse \"index\"");
        return 0;
    }
    // Filename checking among indexes ?

    return 1;
}

int parseLocations(JSON::Object *src, std::map<std::string, Location> &res, Location &base) {
    switch (basicCheck(src, "locations", OBJECT, res, res)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    JSON::Object *locations = src->get("locations")->toObj();

    JSON::Object::iterator it = locations->begin();
    JSON::Object::iterator end = locations->end();
    for (; it != end; it++) {
        Location dst = base;
        if (!basicCheck(locations, it->first, OBJECT)) {
            return 0;
        }

        if (!isValidPath(it->first)) {
            Log.error("### location path \"" + it->first + "\" is incorrect");
            return 0;
        }

        JSON::Object *src = it->second->toObj();
        if (!parseLocation(src, dst, base)) {
            Log.error("### Failed to parse location \"" + it->first + "\"");
            return 0;
        }
        res.insert(std::make_pair(it->first, dst));
    }
    return 1;
}

int parseServerBlock(JSON::Object *src, ServerBlock &dst) {
    if (!getString(src, "server_name", STRING, dst.getServerNameRef(), "")) {
        Log.error("## Failed to parse \"server_name\"");
        return 0;
    }

    if (!getString(src, "addr", STRING, dst.getAddrRef(), "127.0.0.1")) {
        Log.error("## Failed to parse \"addr\"");
        return 0;
    } else if (!isValidIp(dst.getAddrRef())) {
        Log.error("## \"addr\" is invalid or not in ipv4 format");
        return 0;
    }

    if (!getUInteger(src, "port", NUMBER, dst.getPortRef())) {
        Log.error("## Failed to parse \"port\"");
        return 0;
    } else if (dst.getPortRef() < 1024 || dst.getPortRef() > 49151) {
        Log.error("## Port number beyond boundaries");
        return 0;
    }

    if (!parseErrorPages(src, dst.getErrPathsRef())) {
        Log.error("## Failed to parse \"error_pages\"");
        return 0;
    } else if (!isValidErrorPages(dst.getErrPathsRef())) {
        Log.error("## Failed to parse \"error_pages\"");
        return 0;
    }

    dst.getLocationBaseRef().getRootRef() = "./"; // Default path for base root
    if (!parseLocation(src, dst.getLocationBaseRef(), dst.getLocationBaseRef())) {
        Log.error("## Failed to parse \"location base\"");
        return 0;
    }

    if (!parseLocations(src, dst.getLocationsRef(), dst.getLocationBaseRef())) {
        Log.error("## Failed to parse \"locations\"");
        return 0;
    }

    return 1;
}

int parseServerBlocks(JSON::Object *src, Server *serv) {
    switch (basicCheck(src, "servers", OBJECT)) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            return 1;
        default:
            return 0;
    }

    JSON::Object *servers = src->get("servers")->toObj();

    JSON::Object::iterator it = servers->begin();
    JSON::Object::iterator end = servers->end();
    for (; it != end; it++) {
        ServerBlock block_dst;
        block_dst.setBlockname(it->first);

        if (!basicCheck(servers, it->first, OBJECT)) {
            return 0;
        }

        JSON::Object *block_src = it->second->toObj();
        if (!parseServerBlock(block_src, block_dst)) {
            Log.error("# Failed to parse server block \"" + it->first + "\"");
            return 0;
        }
        serv->addServerBlock(block_dst);
    }
    return 1;
}

Server *loadConfig(const string filename) {
    JSON::Object *ptr;
    try {
        JSON::JSON json(filename);
        ptr = json.parse();
        if (ptr == NULL) {
            Log.error("Failed to parse config file");
            return NULL;
        }
    } catch (std::exception &e) {
        Log.error(e.what());
        return NULL;
    }

    Server *serv = new Server();
    if (!parseServerBlocks(ptr, serv)) {
        delete serv;
        serv = NULL;
    } else if (serv && !serv->getServerBlocksNum()) {
        delete serv;
        serv = NULL;
        Log.error("At least one server block needed to start the server.");
    }

    delete ptr;
    return serv;
}
