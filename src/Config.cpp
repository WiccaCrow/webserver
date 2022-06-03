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
getDataTypeName(ExpectedType type) {
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
typeExpected(JSON::AType *ptr, ExpectedType type) {
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
basicCheck(JSON::Object *src, const std::string &key, ExpectedType type, T &res, T def) {
    JSON::AType *ptr = src->get(key);
    if (ptr->isNull()) {
        res = def;
        Log.info("Optional parameter \"" + key + "\" is not found (default used).");
        return DEFAULT;
    }

    if (!typeExpected(ptr, type)) {
        Log.error("\"" + key + "\": expected " + getDataTypeName(type) + ", got " + ptr->getType());
        return NONE_OR_INV;
    }
    return SET;
}

ConfStatus
basicCheck(JSON::Object *src, const std::string &key, ExpectedType type) {
    JSON::AType *ptr = src->get(key);
    if (ptr->isNull()) {
        Log.error("\"" + key + "\" does not exist.");
        return NONE_OR_INV;
    }

    if (!typeExpected(ptr, type)) {
        Log.error("\"" + key + "\": expected " + getDataTypeName(type) + ", got " + ptr->getType());
        return NONE_OR_INV;
    }
    return SET;
}

int
getUInteger(JSON::Object *src, const std::string &key, int &res, int def) {
    ConfStatus status = basicCheck(src, key, NUMBER, res, def);
    if (status != SET) {
        return status;
    }

    double num = src->get(key)->toNum();
    if (isUInteger(num)) {
        res = static_cast<unsigned int>(num);
        return SET;
    } else {
        Log.error(key + ": should be an unsigned integer.");
        return NONE_OR_INV;
    }
}

int
getUInteger(JSON::Object *src, const std::string &key, int &res) {
    ConfStatus status = basicCheck(src, key, NUMBER);
    if (status != SET) {
        return status;
    }

    double num = src->get(key)->toNum();
    if (isUInteger(num)) {
        res = static_cast<unsigned int>(num);
        return SET;
    } else {
        Log.error(key + ": should be an unsigned integer.");
        return NONE_OR_INV;
    }
}

int
getString(JSON::Object *src, const std::string &key, std::string &res, std::string def) {
    ConfStatus status = basicCheck(src, key, STRING, res, def);
    if (status != SET) {
        return status;
    }

    res = src->get(key)->toStr();
    return SET;
}

int
getString(JSON::Object *src, const std::string &key, std::string &res) {
    ConfStatus status = basicCheck(src, key, STRING);
    if (status != SET) {
        return status;
    }

    res = src->get(key)->toStr();
    return SET;
}

int
getBoolean(JSON::Object *src, const std::string &key, bool &res, bool def) {
    ConfStatus status = basicCheck(src, key, BOOLEAN, res, def);
    if (status != SET) {
        return status;
    }

    res = src->get(key)->toBool();
    return SET;
}

int
getBoolean(JSON::Object *src, const std::string &key, bool &res) {
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
getArray(JSON::Object *src, const std::string &key, std::vector<std::string> &res, std::vector<std::string> def) {
    ConfStatus status = basicCheck(src, key, ARRAY, res, def);
    if (status != SET) {
        return status;
    }

    JSON::Array *arr = src->get(key)->toArr();

    // Overwriting inherited values from location_base
    res.clear();

    JSON::Array::iterator it  = arr->begin();
    JSON::Array::iterator end = arr->end();
    for (; it != end; it++) {
        if ((*it)->isNull() || !(*it)->isStr()) {
            Log.error(key + " has mixed value(s)");
            return NONE_OR_INV;
        }
        res.push_back((*it)->toStr());
    }
    return SET;
}

int
getArray(JSON::Object *src, const std::string &key, std::vector<std::string> &res) {
    ConfStatus status = basicCheck(src, key, ARRAY);
    if (status != SET) {
        return status;
    }

    JSON::Array *arr = src->get(key)->toArr();

    JSON::Array::iterator it  = arr->begin();
    JSON::Array::iterator end = arr->end();
    res.clear();
    for (; it != end; it++) {
        if ((*it)->isNull() || !(*it)->isStr()) {
            Log.error(key + " has mixed value(s)");
            return NONE_OR_INV;
        }
        res.push_back((*it)->toStr());
    }
    return SET;
}

// Default values
std::vector<std::string>
getDefaultAllowedMethods() {
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

bool
isValidKeywordLocation(const std::string &key) {
    std::vector<std::string> allowed(8);

    allowed.push_back("CGI");
    allowed.push_back("root");
    allowed.push_back("alias");
    allowed.push_back("methods_allowed");
    allowed.push_back("post_max_body");
    allowed.push_back("autoindex");
    allowed.push_back("index");
    allowed.push_back("redirect");

    if (std::find(allowed.begin(), allowed.end(), key) == allowed.end()) {
        Log.error("Keyword \"" + key + "\" is unrecognized or can't be used in location context");
        return false;
    }
    return true;
}

bool
isValidKeywordServerBlock(const std::string &key) {
    std::vector<std::string> allowed(12);

    allowed.push_back("port");
    allowed.push_back("server_names");
    allowed.push_back("error_pages");
    allowed.push_back("addr");
    allowed.push_back("locations");
    allowed.push_back("CGI");
    allowed.push_back("root");
    allowed.push_back("methods_allowed");
    allowed.push_back("post_max_body");
    allowed.push_back("autoindex");
    allowed.push_back("index");
    allowed.push_back("redirect");

    if (std::find(allowed.begin(), allowed.end(), key) == allowed.end()) {
        Log.error("Keyword \"" + key + "\" is unrecognized or can't be used in serverblock context");
        return false;
    }
    return true;
}

bool
isValidKeywords(JSON::Object *src, bool (*validator)(const std::string &)) {
    JSON::Object::iterator it  = src->begin();
    JSON::Object::iterator end = src->end();

    for (; it != end; it++) {
        if (!validator(it->first)) {
            return false;
        }
    }
    return true;
}

// Object parsing
int
parseCGI(JSON::Object *src, std::map<std::string, HTTP::CGI> &res) {
    ConfStatus status = basicCheck(src, "CGI", OBJECT, res, res);
    if (status != SET) {
        return status;
    }

    JSON::Object *obj = src->get("CGI")->toObj();

    JSON::Object::iterator it  = obj->begin();
    JSON::Object::iterator end = obj->end();
    res.clear();
    for (; it != end; it++) {
        HTTP::CGI cgi;

        std::string value = "";
        if (!getString(obj, it->first, value)) {
            Log.error("\"" + it->first + "\" must be string");
            return NONE_OR_INV;
        }
        cgi.setExecPath(value);
        if (it->first == cgi.compiledExt) {
            cgi.setCompiled(true);
        }
        
        res.insert(std::make_pair(it->first, cgi));
    }
    return SET;
}

int
isValidCGI(std::map<std::string, HTTP::CGI> &res) {
    std::map<std::string, HTTP::CGI>::iterator it  = res.begin();
    std::map<std::string, HTTP::CGI>::iterator end = res.end();

    for (; it != end; it++) {
        if (!isExtension(it->first)) {
            Log.error("\"" + it->first + "\": incorrect extension");
            return false;
        } else if (!it->second.isCompiled() && !isExecutableFile(it->second.getExecPath())) {
            Log.error("\"" + it->second.getExecPath() + "\" is not executable file");
            return false;
        }
    }
    return true;
}

int
parseErrorPages(JSON::Object *src, std::map<int, std::string> &res) {
    ConfStatus status = basicCheck(src, "error_pages", OBJECT, res, res);
    if (status != SET) {
        return status;
    }

    JSON::Object *errObj = src->get("error_pages")->toObj();

    JSON::Object::iterator it  = errObj->begin();
    JSON::Object::iterator end = errObj->end();
    for (; it != end; it++) {
        double value = strtod(it->first.c_str(), NULL);
        if (!isUInteger(value)) {
            Log.error("\"error_pages\" code is not an interger");
            return NONE_OR_INV;
        }
        else if (value < 100 || value > 599) {
            Log.error("\"error_pages\" code " + to_string(value) + " is beyong boundaries");
            return NONE_OR_INV;
        }
        int code = static_cast<int>(value);
        if (it->second->isNull() || !it->second->isStr()) {
            Log.error("\"error_pages\" " + to_string(value) + ": value is not string");
            return NONE_OR_INV;
        }
        res.insert(std::make_pair(code, it->second->toStr()));
    }
    return SET;
}

int
isValidErrorPages(std::map<int, std::string> &res) {
    std::map<int, std::string>::iterator it  = res.begin();
    std::map<int, std::string>::iterator end = res.end();

    for (; it != end; it++) {
        if (!resourceExists(it->second)) {
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

int
parseRedirect(JSON::Object *src, Redirect &res) {
    ConfStatus status = basicCheck(src, "redirect", OBJECT, res, res);
    if (status != SET) {
        return status;
    }

    JSON::Object *rd = src->get("redirect")->toObj();

    if (!getUInteger(rd, "code", res.getCodeRef()))
        return NONE_OR_INV;

    if (!getString(rd, "uri", res.getURIRef()))
        return NONE_OR_INV;

    res.toggle();
    return SET;
}

int
isValidRedirect(Redirect &res) {

    if (res.isSet()) {
        if (res.getCodeRef() < 300 && res.getCodeRef() > 308) {
            Log.error("Redirect code \"" + to_string(res.getCodeRef()) + "\"is invalid");
            return 0;
        } else if (res.getURIRef() == "") {
            Log.error("Redirect uri is empty");
            return 0;
        }
    }
    return 1;
}

int checkMutualExclusions(JSON::Object *src, const std::string &key1, const std::string &key2) {
    JSON::AType *ptr1 = src->get(key1);
    JSON::AType *ptr2 = src->get(key2);
    
    return ptr1->isNull() || ptr2->isNull();
}

int
parseLocation(JSON::Object *src, HTTP::Location &dst, HTTP::Location &def) {
    if (&dst != &def) {

        if (!isValidKeywords(src, isValidKeywordLocation)) {
            return 0;
        }
        if (!checkMutualExclusions(src, "alias", "root")) {
            Log.error("#### \"root\" and \"alias\" are mutually exclusive");
            return 0;
        }

        ConfStatus aliasStatus = (ConfStatus)getString(src, "alias", dst.getAliasRef(), "");
        
        if (aliasStatus == NONE_OR_INV) {
            Log.error("#### Failed to parse \"alias\"");
            return 0;
        }

        if (aliasStatus != DEFAULT) {
            if (!resourceExists(dst.getAliasRef())) {
                Log.error("#### \"alias\": " + dst.getAliasRef() + " does not exist");
                return NONE_OR_INV;
            } else if (!isDirectory(dst.getAliasRef())) {
                Log.error("#### \"alias\" should be a directory");
                return NONE_OR_INV;
            } 
            // else if (dst.getAliasRef()[dst.getAliasRef().length() - 1] != '/') { // ?
            //     dst.getAliasRef() += "/";
            // }
        } 
    }

    if (!getString(src, "root", dst.getRootRef(), def.getRootRef())) {
        Log.error("#### Failed to parse \"root\"");
        return NONE_OR_INV;
    } else if (!resourceExists(dst.getRootRef())) {
        Log.error("#### \"root\": " + dst.getRootRef() + " does not exist");
        return NONE_OR_INV;
    } else if (!isDirectory(dst.getRootRef())) {
        Log.error("#### \"root\" should be a directory");
        return NONE_OR_INV;
    } else if (dst.getRootRef()[dst.getRootRef().length() - 1] != '/') { // ?
        dst.getRootRef() += "/";
    }

    if (!getUInteger(src, "post_max_body", dst.getPostMaxBodyRef(), 200)) {
        Log.error("#### Failed to parse \"post_max_body\"");
        return NONE_OR_INV;
    }

    if (!getBoolean(src, "autoindex", dst.getAutoindexRef(), false)) {
        Log.error("#### Failed to parse \"autoindex\"");
        return NONE_OR_INV;
    }

    if (!parseRedirect(src, dst.getRedirectRef())) {
        Log.error("#### Failed to parse \"redirect\"");
        return NONE_OR_INV;
    } else if (!isValidRedirect(dst.getRedirectRef())) {
        return NONE_OR_INV;
    }

    if (!parseCGI(src, dst.getCGIsRef())) {
        Log.error("#### Failed to parse \"CGI\"");
        return NONE_OR_INV;
    } else if (!isValidCGI(dst.getCGIsRef())) {
        Log.error("#### Invalid \"CGI\". Prototype: \"extension\": \"path-to-executable\"");
        return NONE_OR_INV;
    }

    if (!getArray(src, "methods_allowed", dst.getAllowedMethodsRef(), getDefaultAllowedMethods())) {
        Log.error("#### Failed to parse \"methods_allowed\"");
        return NONE_OR_INV;
    } else if (!isSubset(getDefaultAllowedMethods(), dst.getAllowedMethodsRef())) {
        Log.error("#### Unrecognized value in \"methods_allowed\"");
        return NONE_OR_INV;
    }

    if (!getArray(src, "index", dst.getIndexRef(), def.getIndexRef())) {
        Log.error("#### Failed to parse \"index\"");
        return NONE_OR_INV;
    }

    // Filename checking among indexes ?

    return SET;
}

int
parseLocations(JSON::Object *src, std::map<std::string, HTTP::Location> &res, HTTP::Location &base) {
    ConfStatus status = basicCheck(src, "locations", OBJECT, res, res);
    if (status != SET) {
        return status;
    }

    JSON::Object *locations = src->get("locations")->toObj();

    JSON::Object::iterator it  = locations->begin();
    JSON::Object::iterator end = locations->end();
    for (; it != end; it++) {
        HTTP::Location dst = base;
        if (!basicCheck(locations, it->first, OBJECT)) {
            return NONE_OR_INV;
        }

        if (!isValidPath(it->first)) {
            Log.error("### location path \"" + it->first + "\" is incorrect");
            return NONE_OR_INV;
        }
        dst.getPathRef() = it->first;
        JSON::Object *src = it->second->toObj();
        if (!parseLocation(src, dst, base)) {
            Log.error("### Failed to parse location \"" + it->first + "\"");
            return NONE_OR_INV;
        }
        
        res.insert(std::make_pair(it->first, dst));
    }
    return SET;
}

int
parseServerBlock(JSON::Object *src, HTTP::ServerBlock &dst) {

    if (!isValidKeywords(src, isValidKeywordServerBlock)) {
        return NONE_OR_INV;
    }

    if (!getArray(src, "server_names", dst.getServerNameRef(), dst.getServerNameRef())) {
        Log.error("## Failed to parse \"server_names\"");
        return NONE_OR_INV;
    }

    if (!getString(src, "addr", dst.getAddrRef(), "127.0.0.1")) {
        Log.error("## Failed to parse \"addr\"");
        return NONE_OR_INV;
    } else if (!isValidIp(dst.getAddrRef())) {
        Log.error("## \"addr\" is invalid or not in ipv4 format");
        return NONE_OR_INV;
    }

    if (!getUInteger(src, "port", dst.getPortRef())) {
        Log.error("## Failed to parse \"port\"");
        return NONE_OR_INV;
    } else if (dst.getPortRef() < 1024) {
        Log.info("## WARNING: Ports lower than 1024 reserved for OS");
    } else if ( dst.getPortRef() > 49151) {
        Log.info("## WARNING: Ports higher than 49151 reserved for client apps");
    }

    if (!parseErrorPages(src, dst.getErrPathsRef())) {
        Log.error("## Failed to parse \"error_pages\"");
        return NONE_OR_INV;
    } else if (!isValidErrorPages(dst.getErrPathsRef())) {
        Log.error("## Failed to parse \"error_pages\"");
        return NONE_OR_INV;
    }

    // char resolvedPath[256] = {0};
    // realpath("./", resolvedPath);
    dst.getLocationBaseRef().getRootRef() = "./";
    if (!parseLocation(src, dst.getLocationBaseRef(), dst.getLocationBaseRef())) {
        Log.error("## Failed to parse \"location base\"");
        return NONE_OR_INV;
    }

    if (!parseLocations(src, dst.getLocationsRef(), dst.getLocationBaseRef())) {
        Log.error("## Failed to parse \"locations\"");
        return NONE_OR_INV;
    }

    return SET;
}

int
parseServerBlocks(JSON::Object *src, Server *serv) {
    ConfStatus status = basicCheck(src, "servers", OBJECT);
    if (status != SET) {
        return status;
    }

    JSON::Object *servers = src->get("servers")->toObj();

    JSON::Object::iterator it  = servers->begin();
    JSON::Object::iterator end = servers->end();
    if (it == end) {
        Log.error("At least one server block is needed.");
        return NONE_OR_INV;
    }
    for (; it != end; it++) {
        HTTP::ServerBlock block_dst;
        block_dst.setBlockname(it->first);

        if (!basicCheck(servers, it->first, OBJECT)) {
            return NONE_OR_INV;
        }

        JSON::Object *block_src = it->second->toObj();
        if (!parseServerBlock(block_src, block_dst)) {
            Log.error("# Failed to parse server block \"" + it->first + "\"");
            return NONE_OR_INV;
        }
        serv->addServerBlock(block_dst);
    }
    return SET;
}

Server *
loadConfig(const string filename) {
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
    }

    delete ptr;
    return serv;
}
