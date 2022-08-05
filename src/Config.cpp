#include "Config.hpp"

std::list<std::string> trace;

void conftrace_add(const std::string &kw) {
    trace.push_front(kw);
}

std::string conftrace_path(void) {
    typedef std::list<std::string>::iterator iter;
    std::string fullpath;

    for (iter it = trace.begin(); it != trace.end(); ++it) {
        fullpath += *it + "::";
    }

    return fullpath;
}

int
isInteger(double &num) {
    return (num - static_cast<int32_t>(num) == 0);
}

int
isUInteger(double &num) {
    return (isInteger(num) && num >= 0);
}

std::ostream &operator<<(std::ostream &out, ExpectedType type) {
    out << getExpectedTypeName(type);
    return out;
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
        // Log.info() << "Used default parameter for " << src->getKey() << "::" << key << Log.endl;
        return DEFAULT;
    }

    if (!typeExpected(ptr, type)) {
        Log.error() << key << ": expected " << type << ", got " << ptr->getType() << Log.endl;
        return NONE_OR_INV;
    }
    return SET;
}

ConfStatus
basicCheck(Object *src, const std::string &key, ExpectedType type) {
    AType *ptr = src->get(key);
    if (ptr->isNull()) {
        Log.error() << key << " does not exist." << Log.endl;
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
getUInteger(Object *src, const std::string &key, std::size_t &res, std::size_t def) {

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
getUInteger(Object *src, const std::string &key, std::size_t &res) {

    ConfStatus status = basicCheck(src, key, NUMBER);
    if (status != SET) {
        return status;
    }

    double num = src->get(key)->toNum();
    if (isUInteger(num)) {
        res = static_cast<std::size_t>(num);
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
            Log.error() << key << " has non-string value(s)" << Log.endl;
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
            Log.error() << key << " has non-string value(s)" << Log.endl;
            return NONE_OR_INV;
        }
        res.push_back((*it)->toStr());
    }
    return SET;
}

Location::MethodsVec
getDefaultAllowedMethods(void) {

    Location::MethodsVec allowed(9);
    for (std::size_t i = 0; validMethods[i]; i++) {
        allowed.push_back(validMethods[i]);   
    }

    // allowed.push_back("GET");
    // allowed.push_back("HEAD");   
    // allowed.push_back("POST");
    // allowed.push_back("OPTIONS");

    return allowed;
}

Location::MethodsVec
getDefaultCGIMethods(void) {

    Location::MethodsVec allowed(9);
    allowed.push_back("GET");   
    allowed.push_back("HEAD");   
    allowed.push_back("POST");   

    return allowed;
}

const char * validKeywords[] = {
    KW_LISTEN, KW_SERVER_NAMES, KW_ERROR_PAGES, KW_PROXY_DOMAINS, KW_PROXY_PASS, KW_ADD_HEADERS,
    KW_LOCATIONS, KW_CGI, KW_ROOT, KW_ALIAS, KW_INDEX, KW_AUTOINDEX, 
    KW_METHODS_ALLOWED, KW_POST_MAX_BODY, KW_REDIRECT, KW_AUTH_BASIC,
    KW_SETTINGS, KW_MAX_WAIT_CONN, KW_WORKERS, KW_WORKER_TIMEOUT, KW_MAX_REQUESTS,
    KW_MAX_CLIENT_TIMEOUT, KW_MAX_GATEWAY_TIMEOUT, KW_MAX_URI_LENGTH, 
    KW_MAX_HEADER_FIELD_LENGTH, KW_BLIND_PROXY, KW_SESSION_LIFETIME, KW_CHUNK_SIZE,
    KW_MAX_REG_FILE_SIZE, KW_MAX_RANGE_SIZE, KW_COOKIE_HTTP_ONLY, KW_MAX_REG_UPLOAD_SIZE,
    KW_CGI_METHODS, NULL
};

const char * validSettingsKeywords[] = {
    KW_SETTINGS, KW_MAX_WAIT_CONN, KW_WORKERS, KW_WORKER_TIMEOUT, KW_MAX_REQUESTS,
    KW_MAX_CLIENT_TIMEOUT, KW_MAX_GATEWAY_TIMEOUT, KW_MAX_URI_LENGTH, 
    KW_MAX_HEADER_FIELD_LENGTH, KW_BLIND_PROXY, KW_SESSION_LIFETIME, KW_CHUNK_SIZE,
    KW_MAX_REG_FILE_SIZE, KW_MAX_RANGE_SIZE, KW_COOKIE_HTTP_ONLY, KW_MAX_REG_UPLOAD_SIZE, NULL
};

const char * validServerBlockKeywords[] = {
    KW_LISTEN, KW_SERVER_NAMES,
    KW_LOCATIONS, KW_PROXY_DOMAINS, NULL
};

const char * validLocationKeywords[] = {
    KW_CGI, KW_ROOT, KW_ALIAS, KW_INDEX, KW_AUTOINDEX, KW_ERROR_PAGES, KW_PROXY_PASS,
    KW_METHODS_ALLOWED, KW_POST_MAX_BODY, KW_REDIRECT, KW_AUTH_BASIC, KW_ADD_HEADERS,
    KW_CGI_METHODS, NULL
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

int checkMutualExclusions(Object *src, const std::string &key1, const std::string &key2) {
    AType *ptr1 = src->get(key1);
    AType *ptr2 = src->get(key2);

    bool valid = ptr1->isNull() || ptr2->isNull();

    if (!valid) {
        Log.error() << "\"" << key1 << "\" and \"" << key2 << "\" are mutually exclusive" << Log.endl;
    }
    
    return valid;
}

// Object parsing
int
parsePath(std::string &path, bool addEndSlash = false) {

    if (path.empty()) {
        return 1;
    }

    if (path[0] == '~') {
        std::string home = getenv("HOME");
        if (home[home.length() - 1] == '/') {
            home.erase(home.length() - 1);
        }

        if (path.length() == 1) {
            path = home;
        } else if (path[1] == '/') {
            path = home + path.substr(1);
        }
    }

    if (addEndSlash) {
        if (!endsWith(path, "/")) {
            path += "/";
        }
    } else {
        if (endsWith(path, "/")) {
            path.erase(path.size() - 1);
        }
    }

    return 1;
}

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
        parsePath(value);
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
            Log.error() << KW_ERROR_PAGES << " value " << value << " is not a string" << Log.endl;
            return NONE_OR_INV;
        }
        std::string path = it->second->toStr();
        parsePath(path);
        res.insert(std::make_pair(code, path));
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
    std::size_t code = 0;
    if (!getUInteger(obj, KW_CODE, code)) {
        return NONE_OR_INV;
    }

    if (!getString(obj, KW_URL, res.getURIRef())) {
        return NONE_OR_INV;
    }

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
isValidProxy(URI &proxy) {

    const std::string &host = proxy._host;
    const std::string &port= proxy._port_s;

    if (host.empty()) {
        if (!port.empty()) {
            conftrace_add(KW_PROXY_PASS);
            Log.error() << KW_PROXY_PASS << " contains only port" << Log.endl;
            return 0;
        }
    } else if (!isValidHost(host)) {
        conftrace_add(KW_PROXY_PASS);
        Log.error() << KW_PROXY_PASS << " has invalid host" << Log.endl;
        return 0;
    } else if (!isValidPort(port)) {
        conftrace_add(KW_PROXY_PASS);
        Log.error() << KW_PROXY_PASS << " has invalid port" << Log.endl;
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
        conftrace_add(KW_REALM);
        return NONE_OR_INV;
    }

    if (!getString(obj, KW_USER_FILE, res.getFileRef())) {
        conftrace_add(KW_USER_FILE);
        return NONE_OR_INV;
    }

    res.set(true);
    return SET;
}

int
isValidAuth(Auth &res) {

    if (res.isSet()) {
        if (res.getRealmRef().empty()) {
            conftrace_add(KW_REALM);
            Log.error() << KW_REALM << " is empty" << Log.endl;
            return 0;
        } else if (!resourceExists(res.getFileRef())) {
            conftrace_add(KW_USER_FILE);
            Log.error() << KW_USER_FILE << " " << res.getFileRef() << " does not exist" << Log.endl;
            return 0;
        } else if (!isReadableFile(res.getFileRef())) {
            conftrace_add(KW_USER_FILE);
            Log.error() << KW_USER_FILE << " " << res.getFileRef() << " is not readable" << Log.endl;
            return 0;
        } else if (!res.loadData()) {
            conftrace_add(KW_USER_FILE);
            Log.error() << KW_USER_FILE << " " << res.getFileRef() << " load failed" << Log.endl;
            return 0;
        }
    }
    return 1;
}

int
parseLocation(Object *src, Location &dst, Location &def) {

    bool addEndSlash = endsWith(dst.getPathRef(), "/");

    if (&dst != &def) {

        if (!isValidKeywords(src, validLocationKeywords)) {
            return NONE_OR_INV;
        }
        if (!checkMutualExclusions(src, KW_ALIAS, KW_ROOT)) {
            return NONE_OR_INV;
        }

        ConfStatus aliasStatus = (ConfStatus)getString(src, KW_ALIAS, dst.getAliasRef(), "");
        
        if (aliasStatus == NONE_OR_INV) {
            conftrace_add(KW_ALIAS);
            return NONE_OR_INV;
        }

        parsePath(dst.getAliasRef(), addEndSlash);

        if (aliasStatus != DEFAULT) {
            if (!resourceExists(dst.getAliasRef())) {
                conftrace_add(KW_ALIAS);
                Log.error() << KW_ALIAS << " " << dst.getAliasRef() + " does not exist" << Log.endl;
                return NONE_OR_INV;
            } else if (!isDirectory(dst.getAliasRef())) {
                conftrace_add(KW_ALIAS);
                Log.error() << KW_ALIAS << " must be a directory" << Log.endl;
                return NONE_OR_INV;
            }
        } 
    }

    if (!getString(src, KW_ROOT, dst.getRootRef(), def.getRootRef())) {
        conftrace_add(KW_ROOT);
        return NONE_OR_INV;
    }
    
    parsePath(dst.getRootRef(), addEndSlash);

    if (!resourceExists(dst.getRootRef())) {
        conftrace_add(KW_ROOT);
        Log.error() << KW_ROOT << dst.getRootRef() << " does not exist" << Log.endl;
        return NONE_OR_INV;
    } else if (!isDirectory(dst.getRootRef())) {
        conftrace_add(KW_ROOT);
        Log.error() << KW_ROOT << " must be a directory" << Log.endl;
        return NONE_OR_INV;
    } else if (dst.getRootRef()[dst.getRootRef().length() - 1] != '/') { // ??
        dst.getRootRef() += "/";
    }

    string size_s;
    if (!getString(src, KW_POST_MAX_BODY, size_s, "200B")) {
        conftrace_add(KW_POST_MAX_BODY);
        return NONE_OR_INV;
    } else if (!size_s.empty() && !parseSize(size_s, dst.getPostMaxBodyRef())) {
        conftrace_add(KW_POST_MAX_BODY);
        return NONE_OR_INV;
    } 

    if (!getBoolean(src, KW_AUTOINDEX, dst.getAutoindexRef(), false)) {
        conftrace_add(KW_AUTOINDEX);
        return NONE_OR_INV;
    }

    if (!parseRedirect(src, dst.getRedirectRef())) {
        conftrace_add(KW_REDIRECT);
        return NONE_OR_INV;
    } else if (!isValidRedirect(dst.getRedirectRef())) {
        conftrace_add(KW_REDIRECT);
        return NONE_OR_INV;
    }

    dst.getProxyPassRef().clear();
    std::string pass;
    if (!getString(src, KW_PROXY_PASS, pass, "")) {
        conftrace_add(KW_PROXY_PASS);
        return NONE_OR_INV;
    }
    
    dst.getProxyPassRef().parse(pass);
    if (!isValidProxy(dst.getProxyPassRef())) {
        conftrace_add(KW_PROXY_PASS);
        return NONE_OR_INV;
    }

    if (!parseErrorPages(src, dst.getErrorPagesRef())) {
        conftrace_add(KW_ERROR_PAGES);
        return NONE_OR_INV;
    } else if (!isValidErrorPages(dst.getErrorPagesRef())) {
        conftrace_add(KW_ERROR_PAGES);
        return NONE_OR_INV;
    }

    if (!parseAuth(src, dst.getAuthRef())) {
        conftrace_add(KW_AUTH_BASIC);
        return NONE_OR_INV;
    } else if (!isValidAuth(dst.getAuthRef())) {
        conftrace_add(KW_AUTH_BASIC);
        return NONE_OR_INV;
    }

    if (!parseCGI(src, dst.getCGIsRef())) {
        conftrace_add(KW_CGI);
        return NONE_OR_INV;
    } else if (!isValidCGI(dst.getCGIsRef())) {
        conftrace_add(KW_CGI);
        Log.error() << KW_CGI << " is invalid, usage: <ext>:<path-to-exec>" << Log.endl;
        return NONE_OR_INV;
    }

    if (!getArray(src, KW_METHODS_ALLOWED, dst.getAllowedMethodsRef(), getDefaultAllowedMethods())) {
        conftrace_add(KW_METHODS_ALLOWED);
        return NONE_OR_INV;
    } else if (!isSubset(getDefaultAllowedMethods(), dst.getAllowedMethodsRef())) {
        conftrace_add(KW_METHODS_ALLOWED);
        Log.error() << KW_METHODS_ALLOWED << " has unrecognized value" << Log.endl;
        return NONE_OR_INV;
    }

    if (!getArray(src, KW_CGI_METHODS, dst.getCGIMethodsRef(), getDefaultCGIMethods())) {
        conftrace_add(KW_CGI_METHODS);
        return NONE_OR_INV;
    } else if (!isSubset(getDefaultCGIMethods(), dst.getCGIMethodsRef())) {
        conftrace_add(KW_CGI_METHODS);
        Log.error() << KW_CGI_METHODS << " has unrecognized value" << Log.endl;
        return NONE_OR_INV;
    }
     
    if (!parseHeaders(src, dst.getHeaders())) {
        conftrace_add(KW_ADD_HEADERS);
        return NONE_OR_INV;
    }

    if (!getArray(src, KW_INDEX, dst.getIndexRef(), def.getIndexRef())) {
        conftrace_add(KW_INDEX);
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

    if (!basicCheck(locations, "/", OBJECT, base, base)) {
        conftrace_add("/");
        return NONE_OR_INV;
    }

    Object *rootObj = locations->get("/")->toObj();
    base.getRootRef() = "./";

    if (!rootObj->isNull()) {
        if (!parseLocation(rootObj, base, base)) {
            conftrace_add("/");
            return NONE_OR_INV;
        }
    }

    for (Object::iterator it = locations->begin(); it != locations->end(); it++) {
    
        if (it->first == "/") {
            continue ;
        }

        HTTP::Location location = base;
        if (!basicCheck(locations, it->first, OBJECT)) {
            return NONE_OR_INV;
        }

        if (!isValidPath(it->first)) {
            Log.error() << "Invalid location path" << Log.endl;
            conftrace_add(it->first);
            return NONE_OR_INV;
        }
        location.getPathRef() = it->first;

        Object *obj = it->second->toObj();
        if (!parseLocation(obj, location, base)) {
            conftrace_add(it->first);
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
isValidProxyDomains(ServerBlock &srv) {
    ServerBlock::DomainsVec &domains = srv.getProxyDomainsRef();
    for (size_t i = 0; i < domains.size(); ++i) {
        if (!isValidHost(domains[i])) {
            conftrace_add(KW_PROXY_DOMAINS);
            Log.error() << KW_PROXY_DOMAINS << " " << domains[i] << " is invalid" << Log.endl;
            return 0;
        }
    }
    return 1;
}

int
parseServerBlock(Object *src, ServerBlock &dst) {

    if (!isValidKeywords(src, validServerBlockKeywords)) {
        return NONE_OR_INV;
    }

    if (!getArray(src, KW_SERVER_NAMES, dst.getServerNamesRef(), dst.getServerNamesRef())) {
        conftrace_add(KW_SERVER_NAMES);
        return NONE_OR_INV;
    }

    std::string listen;
    if (!getString(src, KW_LISTEN, listen, "0.0.0.0:8080")) {
        conftrace_add(KW_LISTEN);
        return NONE_OR_INV;
    } 
    URI uri;
    uri.parse(listen);
    if (!isValidIpv4(uri._host)) {
        conftrace_add(KW_LISTEN);
        Log.error() << KW_LISTEN << " is invalid or not in ipv4 format" << Log.endl;
        return NONE_OR_INV;
    } else if (!isValidPort(uri._port)) {
        conftrace_add(KW_LISTEN);
        return NONE_OR_INV;
    }
    dst.getAddrRef() = uri._host;
    dst.getPortRef() = uri._port;

    if (!getArray(src, KW_PROXY_DOMAINS, dst.getProxyDomainsRef(), dst.getProxyDomainsRef())) {
        conftrace_add(KW_PROXY_DOMAINS);
        return NONE_OR_INV;
    } else if (!isValidProxyDomains(dst)) {
        conftrace_add(KW_PROXY_DOMAINS);
        return NONE_OR_INV;
    }

    if (!parseLocations(src, dst.getLocationsRef(), dst.getLocationBaseRef())) {
        conftrace_add(KW_LOCATIONS);
        return NONE_OR_INV;
    }

    return SET;
}

int parseSize(std::string &s, uint64_t &size) {

    trim(s, " ");
    
    if (!std::isdigit(s[0])) {
        Log.error() << "Bad size: " << s << Log.endl;
        return 0;
    }

    char *end = NULL;
    uint64_t res = strtoull(s.c_str(), &end, 10);

    if (end == NULL) {
        Log.error() << "Missing size postfix" << Log.endl;
        return 0;
    }

    while (*end == ' ') {
        end++;
    }

            if (!strcmp(end, "B")) {
        // No koefficient
    } else if (!strcmp(end, "KB")) {
        res = res * KB;
    } else if (!strcmp(end, "MB")) {
        res = res * MB;
    } else if (!strcmp(end, "GB")) {
        res = res * GB;
    } else if (!strcmp(end, "TB")) {
        res = res * TB;
    } else if (!strcmp(end, "PB")) {
        res = res * PB;
    } else if (!strcmp(end, "EB")) {
        res = res * EB;
    } else if (!strcmp(end, "KiB")) {
        res = res * KiB;
    } else if (!strcmp(end, "MiB")) {
        res = res * MiB;
    } else if (!strcmp(end, "GiB")) {
        res = res * GiB;
    } else if (!strcmp(end, "TiB")) {
        res = res * TiB;
    } else if (!strcmp(end, "PiB")) {
        res = res * PiB;
    } else if (!strcmp(end, "EiB")) {
        res = res * EiB;
    } else {
        Log.error() << "Unknown size postfix: \"" << end << "\"" << Log.endl;
        return 0;
    }

    size = res;
    return 1;

}

int parseSettings(Object *src, Settings &sets) {

    Settings def;

    ConfStatus status = basicCheck(src, KW_SETTINGS, OBJECT, sets, def);
    if (status != SET) {
        return status;
    }

    Object *obj = src->get(KW_SETTINGS)->toObj();


    if (!isValidKeywords(obj, validSettingsKeywords)) {
        return NONE_OR_INV;
    }

    if (!getUInteger(obj, KW_MAX_WAIT_CONN, sets.max_wait_conn, def.max_wait_conn)) {
        conftrace_add(KW_MAX_WAIT_CONN);
        return NONE_OR_INV;
    }

    if (!getUInteger(obj, KW_WORKERS, sets.workers, def.workers)) {
        conftrace_add(KW_WORKERS);
        return NONE_OR_INV;
    } else if (sets.workers > 30) {
        conftrace_add(KW_WORKERS);
        Log.error() << KW_WORKERS << " upper bound is 30" << Log.endl;
        return NONE_OR_INV;
    }

    size_t time = 0;
    if (!getUInteger(obj, KW_WORKER_TIMEOUT, time, def.worker_timeout)) {
        conftrace_add(KW_WORKER_TIMEOUT);
        return NONE_OR_INV;
    } else {
        sets.worker_timeout = static_cast<time_t>(time);
    }

    time = 0;
    if (!getUInteger(obj, KW_MAX_CLIENT_TIMEOUT, time, def.max_client_timeout)) {
        conftrace_add(KW_MAX_CLIENT_TIMEOUT);
        return NONE_OR_INV;
    } else {
        sets.max_client_timeout = static_cast<time_t>(time);
    }

    time = 0;
    if (!getUInteger(obj, KW_MAX_GATEWAY_TIMEOUT, time, def.max_gateway_timeout)) {
        conftrace_add(KW_MAX_GATEWAY_TIMEOUT);
        return NONE_OR_INV;
    } else {
        sets.max_gateway_timeout = static_cast<time_t>(time);
    }

    time = 0;
    if (!getUInteger(obj, KW_SESSION_LIFETIME, time, def.session_lifetime)) {
        conftrace_add(KW_SESSION_LIFETIME);
        return NONE_OR_INV;
    } else {
        sets.session_lifetime = static_cast<time_t>(time);
    }

    if (!getUInteger(obj, KW_MAX_REQUESTS, sets.max_requests, def.max_requests)) {
        conftrace_add(KW_MAX_REQUESTS);
        return NONE_OR_INV;
    }

    if (!getUInteger(obj, KW_MAX_URI_LENGTH, sets.max_uri_length, def.max_uri_length)) {
        conftrace_add(KW_MAX_URI_LENGTH);
        return NONE_OR_INV;
    }

    if (!getUInteger(obj, KW_MAX_HEADER_FIELD_LENGTH, sets.max_header_field_length, def.max_header_field_length)) {
        conftrace_add(KW_MAX_HEADER_FIELD_LENGTH);
        return NONE_OR_INV;
    }

    if (!getBoolean(obj, KW_BLIND_PROXY, sets.blind_proxy, def.blind_proxy)) {
        conftrace_add(KW_BLIND_PROXY);
        return NONE_OR_INV;
    }

    if (!getBoolean(obj, KW_COOKIE_HTTP_ONLY, sets.cookie_httpOnly, def.cookie_httpOnly)) {
        conftrace_add(KW_COOKIE_HTTP_ONLY);
        return NONE_OR_INV;
    }

    string size_s;
    if (!getString(obj, KW_CHUNK_SIZE, size_s, "")) {
        conftrace_add(KW_CHUNK_SIZE);
        return NONE_OR_INV;
    } else if (!size_s.empty() && !parseSize(size_s, sets.chunk_size)) {
        conftrace_add(KW_CHUNK_SIZE);
        return NONE_OR_INV;
    } 

    size_s = "";
    if (!getString(obj, KW_MAX_RANGE_SIZE, size_s, "")) {
        conftrace_add(KW_MAX_RANGE_SIZE);
        return NONE_OR_INV;
    } else if (!size_s.empty() && !parseSize(size_s, sets.max_range_size)) {
        conftrace_add(KW_MAX_RANGE_SIZE);
        return NONE_OR_INV;
    } 
    
    size_s = "";
    if (!getString(obj, KW_MAX_REG_FILE_SIZE, size_s, "")) {
        conftrace_add(KW_MAX_REG_FILE_SIZE);
        return NONE_OR_INV;
    } else if (!size_s.empty() && !parseSize(size_s, sets.max_reg_file_size)) {
        conftrace_add(KW_MAX_REG_FILE_SIZE);
        return NONE_OR_INV;
    }

    size_s = "";
    if (!getString(obj, KW_MAX_REG_UPLOAD_SIZE, size_s, "")) {
        conftrace_add(KW_MAX_REG_UPLOAD_SIZE);
        return NONE_OR_INV;
    } else if (!size_s.empty() && !parseSize(size_s, sets.max_reg_upload_size)) {
        conftrace_add(KW_MAX_REG_UPLOAD_SIZE);
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

    std::map< std::string, std::set<std::string> > uniqueNames;
    for (Object::iterator it = obj->begin(); it != obj->end(); it++) {
        ServerBlock servBlock;
        servBlock.setBlockname(it->first);

        if (!basicCheck(obj, it->first, OBJECT)) {
            return NONE_OR_INV;
        }

        if (!parseServerBlock(it->second->toObj(), servBlock)) {
            conftrace_add(it->first);
            return NONE_OR_INV;
        }

        ServerBlock::ServerNamesVec &names = servBlock.getServerNamesRef();
        for (ServerBlock::ServerNamesVec::iterator sn = names.begin(); sn != names.end(); ++sn) {
            const std::string &ip = servBlock.getAddrRef() + ":" + itos(servBlock.getPort());
            if (!uniqueNames[ip].insert(*sn).second) {
                Log.error() << "Duplicated server_name " << *sn << Log.endl;
                conftrace_add(it->first);
                return NONE_OR_INV;
            }
        }
        
        servers[servBlock.getPort()].push_back(servBlock);
    }
    return SET;
}

int
parseConfig(Object *src, Server *serv) {

    if (!parseServerBlocks(src, serv->getServerBlocks())) {
        conftrace_add(KW_SERVERS);
        conftrace_add("conf");
        Log.error() << "at " << conftrace_path() << Log.endl;
        return NONE_OR_INV;
    }

    if (!parseSettings(src, serv->settings)) {
        conftrace_add(KW_SETTINGS);
        conftrace_add("conf");
        Log.error() << "at " << conftrace_path() << Log.endl;
        return NONE_OR_INV;
    }

    return SET;
}

Server *
loadConfig(const string filename) {

    Object *ptr = NULL;
    try {
        ptr = JSON::parseFile(filename);
    } catch (std::exception &e) {
        Log.error() << e.what() << Log.endl;
        return NULL;
    }

    Server *serv = new Server();
    if (serv == NULL) {
        Log.syserr() << "Cannot allocate memory for Server" << Log.endl;
    } else if (!parseConfig(ptr, serv)) {
        delete serv;
        serv = NULL;
    }

    delete ptr;
    return serv;
}


