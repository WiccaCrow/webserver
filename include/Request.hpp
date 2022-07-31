#pragma once

#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

#include "CRC.hpp"
#include "Globals.hpp"
#include "Logger.hpp"
#include "RequestHeader.hpp"
#include "ServerBlock.hpp"
#include "Status.hpp"
#include "URI.hpp"
#include "Utils.hpp"
#include "Time.hpp"
#include "Range.hpp"
#include "ARequest.hpp"

namespace HTTP {

class ServerBlock;
class Location;
class Client;

class Request : public ARequest {

private:
    std::string    _method;
    std::string    _rawURI;

    URI            _uri;
    URI            _host;
    URI            _referrer;

    RangeList      _ranges;

    std::string    _resolvedPath;
    std::string    _remoteUser;
    std::string    _pathInfo;
    
    ServerBlock *  _servBlock;
    Location    *  _location;
    // Client      *  _client;

    bool           _useRanges;
    bool           _authorized;

    std::map<std::string, std::string> _cookie;

public:
    Headers<RequestHeader>  headers;

    Request(void);
    Request(Client *);
    ~Request(void);

    Request(const Request &other);
    Request &operator=(const Request &other);

    ServerBlock *getServerBlock(void) const;
    void         setServerBlock(ServerBlock *);

    Location *getLocation(void) const;
    void      setLocation(Location *);

    const std::string &getRemoteUser(void) const;
    void    setRemoteUser(const std::string &);

    const std::string &getPathInfo(void) const;
    void setPathInfo(const std::string &);

    // virtual bool tunnelGuard(bool);

    virtual bool parseLine(std::string &);

    virtual StatusCode parseSL(const std::string &);
    virtual StatusCode checkSL(void);

    virtual StatusCode parseHeader(const std::string &);
    virtual StatusCode checkHeaders(void);

    virtual StatusCode parseBody(const std::string &);
    virtual StatusCode writeBody(const std::string &);

    const std::string &getPath(void) const;
    const std::string &getMethod(void) const;
    const std::string &getRawUri(void) const;
    
    URI &getUriRef(void);
    URI &getHostRef(void);
    URI &getReferrerRef(void);

    void makeHead(void);
    std::string makeSL(void);

    RangeList &getRangeList(void);

    const std::string &getResolvedPath(void) const;
    void setResolvedPath(const std::string &);

    void resolvePath(void);
    
    bool authorized(void) const;
    void authorized(bool);

    bool useRanges(void) const;
    void useRanges(bool);

    std::map<std::string, std::string>  &getCookie(void);
    void                                setCookie(std::map<std::string, std::string> cookie);

    void proxyLookUp(void);

    void checkCGI(void);

    void addHeader(uint32_t, const std::string & = "");

};

} // namespace HTTP
