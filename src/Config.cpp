#include "Config.hpp"

static std::vector<string> split(string s) {
    size_t beg = 0, end;
    string token;
    std::vector<string> res;
    while ((end = s.find(" ", beg)) != string::npos) {
        token = s.substr(beg, end - beg);
        beg = end + 1;
        res.push_back(token);
    }
    res.push_back(s.substr(beg));
    return res;
}

Server *loadConfig(const string filename) {
    Server *serv = new Server();

    JSON::JSON json(filename);
    JSON::Object *ptr = json.parse();
    if (ptr == NULL) {
        Log.error("# Failed to parse json");
        return NULL;
    }
    if (!parseServerBlocks(ptr, serv)) {
        Log.error("# Failed to parse to server blocks");
        return NULL;
    }

    return serv;
}

int checkInt(double &num) {
    return (num - static_cast<long long>(num) == 0);
}

int checkUInt(double &num) {
    if (!checkInt(num) || num < 0) {
        return 0;
    }
    return 1;
}

int getUintField(JSON::Object *src, const std::string &key, int &value) {
    JSON::AType *ptr = src->get(key);
    if (ptr->isNull()) {
        Log.error(key + " does not exist");
        return 0;
    }
    if (!ptr->isNum()) {
        Log.error(key + " is not a number");
        return 0;
    }
    double num = src->get(key)->toNum();
    if (checkUInt(num)) {
        value = static_cast<int>(num);
    }
    return 1;
}

int getStringField(JSON::Object *src, const std::string &key, std::string &value) {
    JSON::AType *ptr = src->get(key);
    if (ptr->isNull() || !ptr->isStr()) {
        return 0;
    }
    value = src->get(key)->toStr();
    return 1;
}

int getBooleanField(JSON::Object *src, const std::string &key, bool &value) {
    JSON::AType *ptr = src->get(key);
    if (ptr->isNull() || !ptr->isBool()) {
        return 0;
    }
    value = src->get(key)->toBool();
    return 1;
}

int parseServerBlocks(JSON::Object *src, Server *serv) {
    JSON::Object *servers = src->get("servers")->toObj();
    JSON::Object::iterator it = servers->begin();
    JSON::Object::iterator end = servers->end();
    for (; it != end; it++) {
        ServerBlock dst;
        JSON::Object *src = it->second->toObj();
        if (!parseServerBlock(src, dst)) {
            return 0;
        }
        dst.setAddr("127.0.0.1");  
        serv->addServerBlock(dst);
    }
    return 1;
}

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
        if (!checkUInt(value) || value > 999) {
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

int parseAllowedMethods(JSON::Object *src, std::vector<std::string> &res) {
    // methods-allowed
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
    
    JSON::AType *ptr = src->get("methods-allowed");
    if (ptr->isNull()) {
        res = allowed;
    } else if (!ptr->isStr()) {
        return 0;
    } else {
        res = split(ptr->toStr());
        for (size_t i = 0; i < res.size(); i++)
        {
            if (std::find(allowed.begin(), allowed.end(), res[i]) == allowed.end()) {
                return 0;
            }
        }
    }
    return 1;
}

int parseIndex(JSON::Object *src, std::vector<std::string> &res) {
    JSON::AType *ptr = src->get("index");
    if (ptr->isNull()) {
        //
    } else if (!ptr->isStr()) {
        return 0;
    } else {
        res = split(ptr->toStr());
    }
    return 1;
}

int parseLocations(JSON::Object *src, std::map<std::string, Location> &res) {
    JSON::Object *locations = src->get("locations")->toObj();
    JSON::Object::iterator it = locations->begin();
    JSON::Object::iterator end = locations->end();
    for (; it != end; it++) {
        Location dst;
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
    if (!getStringField(src, "root", dst.getRootRef())) {
        Log.error("#### Failed to parse root");
        return 0;
    }
    
    if (!getUintField(src, "post_max_body", dst.getPostMaxBodyRef())) {
        Log.error("#### Failed to parse post_max_body");
        return 0;
    }

    if (!parseCGI(src, dst.getCGIPathsRef())) {
        Log.error("#### Failed to parse cgi");
        return 0;
    }

    if (!parseAllowedMethods(src, dst.getAllowedMethodsRef())) {
        Log.error("#### Failed to parse allowed_methods");
        return 0;
    }

    if (!parseIndex(src, dst.getIndexRef())) {
        Log.error("#### Failed to parse index");
        return 0;
    }
    
    if (!getBooleanField(src, "autoindex", dst.getAutoindexRef())) {
        Log.error("#### Failed to parse autoindex ");
        return 0;
    }
    return 1;
}

int parseServerBlock(JSON::Object *src, ServerBlock &dst) {

    // Basic server attributes
    if (!getStringField(src, "server_name", dst.getServerNameRef())) {
        Log.error("## Failed to parse server_name");
        return 0;
    }
    
    if (!getUintField(src, "port", dst.getPortRef()) || dst.getPortRef() > UINT16_MAX) {
        Log.error("## Failed to parse port");
        return 0;
    }

    if (!parseErrorPages(src, dst.getErrPathsRef())) {
        Log.error("## Failed to parse error_pages");
        return 0;
    }

    if (!parseLocations(src, dst.getLocationsRef())) {
        Log.error("## Failed to parse locations");
        return 0;
    }

    // Location base
    Location &base = dst.getLocationBaseRef();
    if (!getStringField(src, "root", base.getRootRef())) {
        Log.error("## Failed to parse root");
        return 0;
    }

    if (!getUintField(src, "post_max_body", base.getPostMaxBodyRef())) {
        Log.error("## Failed to parse post_max_body");
        return 0;
    }

    if (!getBooleanField(src, "autoindex", base.getAutoindexRef())) {
        Log.error("## Failed to parse autoindex");
        return 0;
    }

    if (!parseCGI(src, base.getCGIPathsRef())) {
        Log.error("## Failed to parse cgi");
        return 0;
    }

    if (!parseAllowedMethods(src, base.getAllowedMethodsRef())) {
        Log.error("## Failed to parse allowed methods");
        return 0;
    }

    if (!parseIndex(src, base.getIndexRef())) {
        Log.error("## Failed to parse index");
        return 0;
    }

    return 1;
}
