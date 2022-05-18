#include "Config.hpp"

int isInteger(double &num) {
    return (num - static_cast<long long>(num) == 0);
}

int isUInteger(double &num) {
    return (isInteger(num) && num >= 0); 
}

enum ExpectedType {
    STRING,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY
};

std::string getDataTypeName(ExpectedType type) {
    switch (type) {
        case ARRAY : return "array";
        case NUMBER : return "number";
        case OBJECT : return "object";
        case STRING : return "string"; 
        case BOOLEAN : return "boolean";
        default: return "unknown";
    }
}

int typeExpected(JSON::AType *ptr, ExpectedType type) {    
    switch (type) {
        case STRING : return ptr->isStr();
        case BOOLEAN : return ptr->isBool();
        case NUMBER : return ptr->isNum();
        case OBJECT : return ptr->isObj();
        case ARRAY : return ptr->isArr();
    }
    return 0;
}


template <typename T>
int baseCheck(JSON::Object *src, const std::string &key, ExpectedType type, T &res, T def) {
    
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

template <typename T>
int baseCheck(JSON::Object *src, const std::string &key, ExpectedType type, T &res) {
    
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
    
    switch (baseCheck(src, key, type, res, def)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
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
    
    switch (baseCheck(src, key, type, res)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
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
    
    switch (baseCheck(src, key, type, res, def)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
    }

    res = src->get(key)->toStr();
    return 1;
}

int getString(JSON::Object *src, const std::string &key, ExpectedType type, std::string &res) {
    
    switch (baseCheck(src, key, type, res)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
    }

    res = src->get(key)->toStr();
    return 1;
}

int getBoolean(JSON::Object *src, const std::string &key, ExpectedType type, bool &res, bool def) {
   
    switch (baseCheck(src, key, type, res, def)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
    }

    res = src->get(key)->toBool();
    return 1;
}

int getBoolean(JSON::Object *src, const std::string &key, ExpectedType type, bool &res) {
   
    switch (baseCheck(src, key, type, res)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
    }

    res = src->get(key)->toBool();
    return 1;
}

template <typename T>
int isSubset(std::vector<T> set, std::vector<T> subset) {
    typename std::vector<T>::iterator it = subset.begin();
    typename std::vector<T>::iterator end = subset.end();
    for (;it != end; it++) {
        if (std::find(set.begin(), set.end(), *it) == set.end()) {
            return 0;
        }
    }
    return 1;
}

int getArray(JSON::Object *src, const std::string &key, ExpectedType type, std::vector<std::string> &res, std::vector<std::string> def) {
    
    switch (baseCheck(src, key, type, res, def)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
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
    
    switch (baseCheck(src, key, type, res)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
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
    JSON::Object *obj = src->get("CGI")->toObj();
    JSON::Object::iterator it = obj->begin();
    JSON::Object::iterator end = obj->end();
    for (; it != end; it++) {
        if (it->first[0] != '.') {
            // Invalid extension in CGI
            return 0;
        }
        std::string value = it->second->toStr();
        res.insert(std::make_pair(it->first, value));
    }
    return 1;
}

int parseErrorPages(JSON::Object *src, std::map<int, std::string> &res) {
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

int parseLocations(JSON::Object *src, std::map<std::string, Location> &res) {
    JSON::Object *locations = src->get("locations")->toObj();
    JSON::Object::iterator it = locations->begin();
    JSON::Object::iterator end = locations->end();
    for (; it != end; it++) {
        Location dst;
        // Check path of location
        JSON::Object *src = it->second->toObj();        
        if (!parseLocation(src, dst)) {
            Log.error("### Failed to parse location \"" + it->first + "\"");
            return 0;
        }
        res.insert(std::make_pair(it->first, dst));
    }
    return 1;
}

int parseLocation(JSON::Object *src, Location &dst) {
    if (!getString(src, "root", STRING, dst.getRootRef(), "/")) { // optional ?
        Log.error("#### Failed to parse \"root\"");
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

    if (!parseCGI(src, dst.getCGIPathsRef())) {
        Log.error("#### Failed to parse \"CGI\"");
        return 0;
    }

    if (!getArray(src, "methods-allowed", ARRAY, dst.getAllowedMethodsRef(), getDefaultAllowedMethods())) {
        Log.error("#### Failed to parse \"methods-allowed\"");
        return 0;
    } 
    else if (!isSubset(getDefaultAllowedMethods(), dst.getAllowedMethodsRef())) {
        Log.error("#### Unrecognized value in \"methods-allowed\"");
        return 0;
    }

    if (!getArray(src, "index", ARRAY, dst.getAllowedMethodsRef(), getDefaultIndex())) {
        Log.error("#### Failed to parse \"index\"");
        return 0;
    }
    // Filename checking among indexes

    return 1;
}

int parseServerBlock(JSON::Object *src, ServerBlock &dst) {

    // Basic server attributes
    if (!getString(src, "server_name", STRING, dst.getServerNameRef(), "")) {
        Log.error("## Failed to parse \"server_name\"");
        return 0;
    }

    if (!getString(src, "addr", STRING, dst.getAddrRef(), "127.0.0.1")) {
        Log.error("## Failed to parse \"addr\"");
        return 0;
    }
    
    if (!getUInteger(src, "port", NUMBER, dst.getPortRef())) {
        Log.error("## Failed to parse \"port\"");
        return 0;
    }
    else if (dst.getPortRef() > UINT16_MAX) {
        Log.error("## Port number exceeds limits");
        return 0;        
    }

    if (!parseErrorPages(src, dst.getErrPathsRef())) {
        Log.error("## Failed to parse \"error_pages\"");
        return 0;
    }

    if (!parseLocation(src, dst.getLocationBaseRef())) {
        Log.error("## Failed to parse \"location base\"");
        return 0;
    }

    if (!parseLocations(src, dst.getLocationsRef())) {
        Log.error("## Failed to parse \"locations\"");
        return 0;
    }

    return 1;
}

int parseServerBlocks(JSON::Object *src, Server *serv) {

    switch (baseCheck(src, "servers", OBJECT, serv)) {
        case 0: return 0;
        case 1: break;
        case 2: return 1;
        default: return 0;
    }

    JSON::Object *servers = src->get("servers")->toObj();
    JSON::Object::iterator it = servers->begin();
    JSON::Object::iterator end = servers->end();
    for (; it != end; it++) {
        ServerBlock dst;        
        dst.setBlockname(it->first);

        JSON::Object *src = it->second->toObj();
        if (!parseServerBlock(src, dst)) {
            Log.error("# Failed to parse server block \"" + it->first + "\"");
            return 0;
        }
        serv->addServerBlock(dst);
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
        delete ptr;
        delete serv;
        return NULL;
    }
    delete ptr;
    return serv;
}


// switch (expression)
// {
// case 0: // Not exist                                            /// Continue
// case 1: // Exist                                                /// Continue

// case 2: // Not exist and optional                               /// Default
// case 3: // Exist and optional                                   /// Continue

// case 4: // Not exist, not optional, but formatted               /// Impossible
// case 5: // Exist, not optional and formatted                    /// Continue
// case 6: // Not exist, optional and formatted                    /// Default
// case 7: // Exist, optional and formatted                        /// Continue

// case 8: // Not exist, not optional, not formatted, but valid    /// Impossible
// case 9: // Exist, not optional, not formatted, but valid        /// Error (bad format)
// case 10: // Not exist, optional, not formatted, but valid       /// Error (incorrect default value)
// case 11: // Exist, optional, not formatted, but valid           /// Error (bad format)
// case 12: // Not exist, not optional, formatted and valid        /// Impossible 
// case 13: // Exist, not optional, formatted and valid            /// OK
// case 14: // Not exist, optional, formatted and valid            /// Default
// case 15: // Exist, optional, formatted and valid                /// OK
// }
