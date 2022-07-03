#include "Config.hpp"

int
isInteger(double &num) {
    return (num - static_cast<int32_t>(num) == 0);
}

int
isUInteger(double &num) {
    return (isInteger(num) && num >= 0);
}

std::string
getExpectedTypeName(ExpectedType type) {
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

int
typeExpected(AType *ptr, ExpectedType type) {
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
ConfStatus
basicCheck(Object *src, const std::string &key, ExpectedType type, T &res, T def) {
    AType *ptr = src->get(key);
    if (ptr->isNull()) {
        res = def;
        Log.info() << "Used default parameter for " << src->getKey() << "::" << key << Log.endl;
        return DEFAULT;
    }

    if (!typeExpected(ptr, type)) {
        Log.error() << key << ": expected " << getExpectedTypeName(type) << Log.endl;
        Log.error() << key << ": got " << ptr->getType() << Log.endl;
        return NONE_OR_INV;
    }
    return SET;
}

ConfStatus
basicCheck(Object *src, const std::string &key, ExpectedType type) {
    AType *ptr = src->get(key);
    if (ptr->isNull()) {
        Log.error() << key << "does not exist." << Log.endl;
        return NONE_OR_INV;
    }

    if (!typeExpected(ptr, type)) {
        Log.error() << key << ": expected " << getExpectedTypeName(type) << Log.endl;
        Log.error() << key << ": got " << ptr->getType() << Log.endl;
        return NONE_OR_INV;
    }
    return SET;
}

int
getUInteger(Object *src, const std::string &key, int &res, int def) {
    ConfStatus status = basicCheck(src, key, NUMBER, res, def);
    if (status != SET) {
        return status;
    }

    double num = src->get(key)->toNum();
    if (isUInteger(num)) {
        res = static_cast<unsigned int>(num);
        return SET;
    } else {
        Log.error() << key << ": should be an unsigned integer." << Log.endl;
        return NONE_OR_INV;
    }
}

int
getUInteger(Object *src, const std::string &key, int &res) {
    ConfStatus status = basicCheck(src, key, NUMBER);
    if (status != SET) {
        return status;
    }

    double num = src->get(key)->toNum();
    if (isUInteger(num)) {
        res = static_cast<unsigned int>(num);
        return SET;
    } else {
        Log.error() << key << ": should be an unsigned integer." << Log.endl;
        return NONE_OR_INV;
    }
}

int
getString(Object *src, const std::string &key, std::string &res, std::string def) {
    ConfStatus status = basicCheck(src, key, STRING, res, def);
    if (status != SET) {
        return status;
    }

    res = src->get(key)->toStr();
    return SET;
}

int
getString(Object *src, const std::string &key, std::string &res) {
    ConfStatus status = basicCheck(src, key, STRING);
    if (status != SET) {
        return status;
    }

    res = src->get(key)->toStr();
    return SET;
}

int
getBoolean(Object *src, const std::string &key, bool &res, bool def) {
    ConfStatus status = basicCheck(src, key, BOOLEAN, res, def);
    if (status != SET) {
        return status;
    }

    res = src->get(key)->toBool();
    return SET;
}

int
getBoolean(Object *src, const std::string &key, bool &res) {
    ConfStatus status = basicCheck(src, key, BOOLEAN);
    if (status != SET) {
        return status;
    }

    res = src->get(key)->toBool();
    return SET;
}

template <typename T>
int
isSubset(std::vector<T> set, std::vector<T> subset) {
    typename std::vector<T>::iterator it  = subset.begin();
    typename std::vector<T>::iterator end = subset.end();
    for (; it != end; it++) {
        if (std::find(set.begin(), set.end(), *it) == set.end()) {
            return 0;
        }
    }
    return 1;
}

int
getArray(Object *src, const std::string &key, std::vector<std::string> &res, std::vector<std::string> def) {
    ConfStatus status = basicCheck(src, key, ARRAY, res, def);
    if (status != SET) {
        return status;
    }

    Array *arr = src->get(key)->toArr();

    // Overwriting inherited values from location_base
    res.clear();

    Array::iterator it  = arr->begin();
    Array::iterator end = arr->end();
    for (; it != end; it++) {
        if ((*it)->isNull() || !(*it)->isStr()) {
            Log.error() << key << " has mixed value(s)" << Log.endl;
            return NONE_OR_INV;
        }
        res.push_back((*it)->toStr());
    }
    return SET;
}

int
getArray(Object *src, const std::string &key, std::vector<std::string> &res) {
    ConfStatus status = basicCheck(src, key, ARRAY);
    if (status != SET) {
        return status;
    }

    Array *arr = src->get(key)->toArr();

    Array::iterator it  = arr->begin();
    Array::iterator end = arr->end();
    res.clear();
    for (; it != end; it++) {
        if ((*it)->isNull() || !(*it)->isStr()) {
            Log.error() << key << " has mixed value(s)" << Log.endl;
            return NONE_OR_INV;
        }
        res.push_back((*it)->toStr());
    }
    return SET;
}

Location::MethodsVec
getDefaultAllowedMethods() {

    Location::MethodsVec allowed(9);
    for (std::size_t i = 0; validMethods[i]; i++) {
        allowed.push_back(validMethods[i]);   
    }

    return allowed;
}

const char * validKeywords[] = {
    KW_LISTEN, KW_SERVER_NAMES, KW_ERROR_PAGES, KW_PROXY, KW_ADD_HEADERS,
    KW_LOCATIONS, KW_CGI, KW_ROOT, KW_ALIAS, KW_INDEX, KW_AUTOINDEX, 
    KW_METHODS_ALLOWED, KW_POST_MAX_BODY, KW_REDIRECT, KW_AUTH_BASIC, NULL
};

const char * validServerBlockKeywords[] = {
    KW_LISTEN, KW_SERVER_NAMES, KW_ERROR_PAGES, KW_ADD_HEADERS, 
    KW_LOCATIONS, KW_CGI, KW_ROOT, KW_INDEX, KW_AUTOINDEX, KW_PROXY,
    KW_METHODS_ALLOWED, KW_POST_MAX_BODY, KW_REDIRECT, KW_AUTH_BASIC, NULL
};

const char * validLocationKeywords[] = {
    KW_CGI, KW_ROOT, KW_ALIAS, KW_INDEX, KW_AUTOINDEX, KW_ERROR_PAGES, KW_PROXY,
    KW_METHODS_ALLOWED, KW_POST_MAX_BODY, KW_REDIRECT, KW_AUTH_BASIC, KW_ADD_HEADERS, NULL
};

const char * validProxyKeywords[] = {
    KW_DOMAINS, KW_PASS, NULL
};

const char * validRedirectKeywords[] = {
    KW_CODE, KW_URL, NULL
};

const char * validAuthBasicKeywords[] = {
    KW_REALM, KW_USER_FILE, NULL
};

bool
isValidKeyword(const std::string &key, const char *contextKeywords[]) {

    for (std::size_t i = 0; contextKeywords[i]; i++) {
        if (contextKeywords[i] == key) {
            return true;
        }
    }

    for (std::size_t i = 0; validKeywords[i]; i++) {
        if (validKeywords[i] == key) {
            Log.error() << "Invalid context for keyword " << key << Log.endl;
            return false;
        }
    }

    Log.error() << "Unrecognized keyword " << key << Log.endl;
    return false;
}

bool
isValidKeywords(Object *src, const char *validKeywords[]) {

    for (Object::iterator it = src->begin(); it != src->end(); ++it) {
        if (!isValidKeyword(it->first, validKeywords)) {
            return false;
        }
    }
    return true;
}


// Object parsing
int
parseCGI(Object *src, Location::CGIsMap &res) {
    
    Location::CGIsMap def;
    res.clear();

    const std::string &key = KW_CGI;

    ConfStatus status = basicCheck(src, key, OBJECT, res, def);
    if (status != SET) {
        return status;
    }

    Object *obj = src->get(key)->toObj();

    for (Object::iterator it = obj->begin(); it != obj->end(); it++) {
        CGI cgi;

        std::string value;
        if (!getString(obj, it->first, value)) {
            return NONE_OR_INV;
        }
        cgi.setExecPath(value);
        if (it->first == cgi.compiledExt) {
            cgi.compiled(true);
        }
        
        res.insert(std::make_pair(it->first, cgi));
    }
    return SET;
}

int
isValidCGI(Location::CGIsMap &res) {

    for (Location::CGIsMap::iterator it = res.begin(); it != res.end(); it++) {
        if (!isExtension(it->first)) {
            Log.error() << it->first << ": incorrect extension" << Log.endl;
            return false;

        } else if (!it->second.compiled() && !isExecutableFile(it->second.getExecPath())) {
            Log.error() << it->second.getExecPath() << " is not an executable file" << Log.endl;
            return false;
        }
    }
    return true;
}

int
parseHeaders(Object *src, Headers<ResponseHeader> &res) {
    std::vector<std::string> headers;

    if (!getArray(src, KW_ADD_HEADERS, headers, headers)) {
        return NONE_OR_INV;
    }

    for (size_t i = 0; i < headers.size(); i++) {
        ResponseHeader header;

        if (!header.parse(headers[i])) {
            Log.error() << "Invalid header: " << headers[i] << Log.endl; 
            return NONE_OR_INV;
        }
        if (validResHeaders.find(header.hash) == validResHeaders.end()) {
            Log.error() << "Non-HTTP header detected: " << headers[i] << Log.endl; 
            return NONE_OR_INV;
        }
        res.insert(header);
    }

    return SET;
}

int
parseErrorPages(Object *src, Location::ErrorPagesMap &res) {

    Location::ErrorPagesMap def;
    res.clear();

    ConfStatus status = basicCheck(src, KW_ERROR_PAGES, OBJECT, res, def);
    if (status != SET) {
        return status;
    }

    Object *obj = src->get(KW_ERROR_PAGES)->toObj();

    for (Object::iterator it = obj->begin(); it != obj->end(); ++it) {

        double value = strtod(it->first.c_str(), NULL);
        if (!isUInteger(value)) {
            Log.error() << KW_ERROR_PAGES << " code is not an positive interger" << Log.endl;
            return NONE_OR_INV;
        } else if (value < 300 || value > 599) {
            Log.error() << KW_ERROR_PAGES << " code " << value << " is beyong boundaries" << Log.endl;
            return NONE_OR_INV;
        }

        int code = static_cast<int>(value);
        if (it->second->isNull() || !it->second->isStr()) {
            Log.error() << KW_ERROR_PAGES << " value " << value <<  " is not a string" << Log.endl;
            return NONE_OR_INV;
        }

        res.insert(std::make_pair(code, it->second->toStr()));
    }
    return SET;
}

int
isValidErrorPages(Location::ErrorPagesMap &res) {

    for (Location::ErrorPagesMap::iterator it = res.begin(); it != res.end(); ++it) {
        if (!resourceExists(it->second)) {
            Log.error() << it->second << ": file does not exist" << Log.endl;
            return false;
        }
        if (!isReadableFile(it->second)) {
            Log.error() << it->second + ": is not readable file" << Log.endl;
            return false;
        }
    }
    return true;
}

int
parseRedirect(Object *src, Redirect &res) {
    Redirect def;

    ConfStatus status = basicCheck(src, KW_REDIRECT, OBJECT, res, def);
    if (status != SET) {
        return status;
    }

    Object *obj = src->get(KW_REDIRECT)->toObj();
    int code = 0;
    if (!getUInteger(obj, KW_CODE, code))
        return NONE_OR_INV;

    if (!getString(obj, KW_URL, res.getURIRef()))
        return NONE_OR_INV;

    res.getCodeRef() = static_cast<StatusCode>(code);
    res.set(true);
    return SET;
}

int
isValidRedirect(Redirect &res) {

    if (res.set()) {
        if (res.getCodeRef() < 300 && res.getCodeRef() > 308) {
            Log.error() << KW_REDIRECT << "::" << KW_CODE << " " << res.getCodeRef() << " is invalid" << Log.endl;
            return 0;
        } else if (res.getURIRef().empty()) {
            Log.error() << KW_REDIRECT << "::" << KW_URL << " is empty" << Log.endl;
            return 0;
        }
    }
    return 1;
}

int
parseProxy(Object *src, Proxy &res) {
    Proxy def;

    ConfStatus status = basicCheck(src, KW_PROXY, OBJECT, res, def);
    if (status != SET) {
        return status;
    }

    Object *obj = src->get(KW_PROXY)->toObj();

    if (!getArray(obj, KW_DOMAINS, res.getDomainsRef())) {
        return NONE_OR_INV;
    }

    std::string pass;
    if (!getString(obj, KW_PASS, pass)) {
        return NONE_OR_INV;
    }
    res.getPassRef().parse(pass);

    return SET;
}

int
isValidProxy(Proxy &res) {

    Proxy::DomainsVec &domains = res.getDomainsRef();
    for (size_t i = 0; i < domains.size(); ++i) {
        if (!isValidHost(domains[i])) {
            Log.error() << KW_DOMAINS << " " << domains[i] << " is invalid" << Log.endl;
            return 0;
        }
    }

    const std::string &host = res.getPassRef()._host;
    const std::string &port= res.getPassRef()._port_s;

    if (host.empty()) {
        if (!port.empty()) {
            Log.error() << KW_PASS << " contains only port" << Log.endl;
            return 0;
        }
    } else if (!isValidHost(host)) {
        Log.error() << KW_PASS << " has invalid host" << Log.endl;
        return 0;
    } else if (!isValidPort(port)) {
        Log.error() << KW_PASS << " has invalid port" << Log.endl;
        return 0;
    }
    return 1;
}

int
parseAuth(Object *src, Auth &res) {

    ConfStatus status = basicCheck(src, KW_AUTH_BASIC, OBJECT, res, res);
    if (status != SET) {
        return status;
    }

    Object *obj = src->get(KW_AUTH_BASIC)->toObj();

    if (!getString(obj, KW_REALM, res.getRealmRef())) {
        return NONE_OR_INV;
    }

    if (!getString(obj, KW_USER_FILE, res.getFileRef())) {
        return NONE_OR_INV;
    }

    res.set(true);
    return SET;
}

int
isValidAuth(Auth &res) {

    if (res.isSet()) {
        if (res.getRealmRef().empty()) {
            Log.error() << KW_REALM << " is empty" << Log.endl;
            return 0;
        } else if (!resourceExists(res.getFileRef())) {
            Log.error() << KW_USER_FILE << " " << res.getFileRef() << " does not exist" << Log.endl;
            return 0;
        } else if (!isReadableFile(res.getFileRef())) {
            Log.error() << KW_USER_FILE << " " << res.getFileRef() << " is not readable" << Log.endl;
            return 0;
        } else if (!res.loadData()) {
            Log.error() << KW_USER_FILE << " " << res.getFileRef() << " load failed" << Log.endl;
            return 0;
        }
    }
    return 1;
}

int checkMutualExclusions(Object *src, const std::string &key1, const std::string &key2) {
    AType *ptr1 = src->get(key1);
    AType *ptr2 = src->get(key2);
    
    return ptr1->isNull() || ptr2->isNull();
}

int
parseLocation(Object *src, Location &dst, Location &def) {

    if (&dst != &def) {

        if (!isValidKeywords(src, validLocationKeywords)) {
            return NONE_OR_INV;
        }
        if (!checkMutualExclusions(src, KW_ALIAS, KW_ROOT)) {
            Log.error() << KW_ROOT << " and " << KW_ALIAS " are mutually exclusive" << Log.endl;
            return NONE_OR_INV;
        }

        ConfStatus aliasStatus = (ConfStatus)getString(src, KW_ALIAS, dst.getAliasRef(), "");
        
        if (aliasStatus == NONE_OR_INV) {
            Log.error() << KW_ALIAS << " parsing failed" << Log.endl;
            return NONE_OR_INV;
        }

        if (aliasStatus != DEFAULT) {
            if (!resourceExists(dst.getAliasRef())) {
                Log.error() << KW_ALIAS << " " << dst.getAliasRef() + " does not exist" << Log.endl;
                return NONE_OR_INV;
            } else if (!isDirectory(dst.getAliasRef())) {
                Log.error() << KW_ALIAS << " must be a directory" << Log.endl;
                return NONE_OR_INV;
            }
        } 
    }

    if (!getString(src, KW_ROOT, dst.getRootRef(), def.getRootRef())) {
        Log.error() << KW_ROOT << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } else if (!resourceExists(dst.getRootRef())) {
        Log.error() << KW_ROOT << dst.getRootRef() << " does not exist" << Log.endl;
        return NONE_OR_INV;
    } else if (!isDirectory(dst.getRootRef())) {
        Log.error() << KW_ROOT << " must be a directory" << Log.endl;
        return NONE_OR_INV;
    } else if (dst.getRootRef()[dst.getRootRef().length() - 1] != '/') { // ??
        dst.getRootRef() += "/";
    }

    if (!getUInteger(src, KW_POST_MAX_BODY, dst.getPostMaxBodyRef(), 200)) {
        Log.error() << KW_POST_MAX_BODY << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    if (!getBoolean(src, KW_AUTOINDEX, dst.getAutoindexRef(), false)) {
        Log.error() << KW_AUTOINDEX << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    if (!parseRedirect(src, dst.getRedirectRef())) {
        Log.error() << KW_REDIRECT << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } else if (!isValidRedirect(dst.getRedirectRef())) {
        return NONE_OR_INV;
    }

    if (!parseProxy(src, dst.getProxyRef())) {
        Log.error() << KW_PROXY << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } else if (!isValidProxy(dst.getProxyRef())) {
        Log.error() << KW_PROXY << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    if (!parseErrorPages(src, dst.getErrorPagesRef())) {
        Log.error() << KW_ERROR_PAGES << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } else if (!isValidErrorPages(dst.getErrorPagesRef())) {
        Log.error() << KW_ERROR_PAGES << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    if (!parseAuth(src, dst.getAuthRef())) {
        Log.error() << KW_AUTH_BASIC << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } else if (!isValidAuth(dst.getAuthRef())) {
        return NONE_OR_INV;
    }

    if (!parseCGI(src, dst.getCGIsRef())) {
        Log.error() << KW_CGI << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } else if (!isValidCGI(dst.getCGIsRef())) {
        Log.error() << KW_CGI << " is invalid, usage: <ext>:<path-to-exec>" << Log.endl;
        return NONE_OR_INV;
    }

    if (!getArray(src, KW_METHODS_ALLOWED, dst.getAllowedMethodsRef(), getDefaultAllowedMethods())) {
        Log.error() << KW_METHODS_ALLOWED << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } else if (!isSubset(getDefaultAllowedMethods(), dst.getAllowedMethodsRef())) {
        Log.error() << KW_METHODS_ALLOWED << " has unrecognized value" << Log.endl;
        return NONE_OR_INV;
    }
     
    if (!parseHeaders(src, dst.getHeaders())) {
        Log.error() << KW_ADD_HEADERS << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    if (!getArray(src, KW_INDEX, dst.getIndexRef(), def.getIndexRef())) {
        Log.error() << KW_INDEX << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    return SET;
}

int
parseLocations(Object *src, ServerBlock::LocationsMap &res, Location &base) {
    ConfStatus status = basicCheck(src, KW_LOCATIONS, OBJECT, res, res);
    if (status != SET) {
        return status;
    }

    Object *locations = src->get(KW_LOCATIONS)->toObj();

    for (Object::iterator it = locations->begin(); it != locations->end(); it++) {
    
        HTTP::Location location = base;
        if (!basicCheck(locations, it->first, OBJECT)) {
            return NONE_OR_INV;
        }

        if (!isValidPath(it->first)) {
            Log.error() << "location " << it->first << " invalid path" << Log.endl;
            return NONE_OR_INV;
        }
        location.getPathRef() = it->first;

        Object *obj = it->second->toObj();
        if (!parseLocation(obj, location, base)) {
            Log.error() << "location " << it->first << " parsing failed" << Log.endl;
            return NONE_OR_INV;
        }
        
        res.insert(std::make_pair(it->first, location));
    }
    return SET;
}

bool
isValidPort(int port) {
    if (port > 65535) {
        Log.error() << "port's upper bound is 65535" << Log.endl;
        return false;
    } else if (port > 49151) {
        Log.info() << "ports 49151-65535 reserved for client apps" << Log.endl;
    } else if (port < 1024 && getuid() != 0) {
        Log.error() << "ports 0-1023 reserved for OS-daemons in unix (use sudo)" << Log.endl;
        return false;
    }
    return true;
}

int
parseServerBlock(Object *src, ServerBlock &dst) {

    if (!isValidKeywords(src, validServerBlockKeywords)) {
        return NONE_OR_INV;
    }

    if (!getArray(src, KW_SERVER_NAMES, dst.getServerNamesRef(), dst.getServerNamesRef())) {
        Log.error() << KW_SERVER_NAMES << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    std::string listen;
    if (!getString(src, KW_LISTEN, listen, "0.0.0.0:8080")) {
        Log.error() << KW_LISTEN << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    } 
    URI uri;
    uri.parse(listen);
    if (!isValidIpv4(uri._host)) {
        Log.error() << KW_LISTEN << " is invalid or not in ipv4 format" << Log.endl;
        return NONE_OR_INV;
    } else if (!isValidPort(uri._port)) {
        Log.error() << KW_LISTEN << " is invalid port" << Log.endl;
        return NONE_OR_INV;
    }
    dst.getAddrRef() = uri._host;
    dst.getPortRef() = uri._port;

    // char resolvedPath[256] = {0};
    // realpath("./", resolvedPath);
    dst.getLocationBaseRef().getRootRef() = "./";
    if (!parseLocation(src, dst.getLocationBaseRef(), dst.getLocationBaseRef())) {
        Log.error() << "location_base" << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    if (!parseLocations(src, dst.getLocationsRef(), dst.getLocationBaseRef())) {
        Log.error() << KW_LOCATIONS << " parsing failed" << Log.endl;
        return NONE_OR_INV;
    }

    return SET;
}

int
parseServerBlocks(Object *src, Server::ServersMap &servers) {

    ConfStatus status = basicCheck(src, KW_SERVERS, OBJECT);
    if (status != SET) {
        return status;
    }

    Object *obj = src->get(KW_SERVERS)->toObj();

    if (obj->begin() == obj->end()) {
        Log.error() << "Serverblocks not found" << Log.endl;
        return NONE_OR_INV;
    }

    for (Object::iterator it = obj->begin(); it != obj->end(); it++) {
        ServerBlock servBlock;
        servBlock.setBlockname(it->first);

        if (!basicCheck(obj, it->first, OBJECT)) {
            return NONE_OR_INV;
        }

        if (!parseServerBlock(it->second->toObj(), servBlock)) {
            Log.error() << "Serverblock " << it->first << " parsing failed" << Log.endl;
            return NONE_OR_INV;
        }
        servers[servBlock.getPort()].push_back(servBlock);
    }
    return SET;
}

Server *
loadConfig(const string filename) {

    Object *ptr;
    try {
        Parser json(filename);
        ptr = json.parse();
        if (ptr == NULL) {
            Log.error() << "Failed to parse " << filename << Log.endl;
            return NULL;
        }
    } catch (std::exception &e) {
        Log.error() << e.what() << " " << filename <<  Log.endl;
        return NULL;
    }

    Server *serv = new Server();
    if (serv == NULL) {
        Log.syserr() << "Cannot allocate memory for Server" << Log.endl;
    } else if (!parseServerBlocks(ptr, serv->getServerBlocks())) {
        delete serv;
        serv = NULL;
    }

    delete ptr;
    return serv;
}
