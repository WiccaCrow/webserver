#include <string>
#include <algorithm>

struct Uri
{
public:
std::string queryString, path, scheme, host, port, fragment;

static Uri Parse(std::string uri)
{
    Uri result;

    typedef std::string::iterator iter_t;

    if (uri.length() == 0)
        return result;

    iter_t fragmentStart = std::find(uri.begin(), uri.end(), '#');
    if (fragmentStart != uri.end()) {
        result.fragment = std::string(fragmentStart, uri.end());
    }

    iter_t queryStart = std::find(uri.begin(), fragmentStart, '?');
    if (queryStart != uri.end()) {
        result.queryString = std::string(queryStart, fragmentStart);
    }

    // scheme    
    size_t pos = uri.find("://");
    iter_t schemeEnd = uri.begin();
    if (pos != std::string::npos) {
        std::advance(schemeEnd, pos);
        result.scheme = std::string(uri.begin(), schemeEnd);
        std::advance(schemeEnd, 3);
    }

    // host
    iter_t pathStart = std::find(schemeEnd, uri.end(), '/');
    if (pathStart != schemeEnd) {

        iter_t authEnd = pathStart != uri.end() ? pathStart : queryStart;
        iter_t hostEnd = std::find(schemeEnd, authEnd, ':');
        
        result.host = std::string(schemeEnd, hostEnd);

        // port
        if (hostEnd != authEnd) {
            std::advance(hostEnd, 1);
            result.port = std::string(hostEnd, authEnd);
        }
    }

    // path
    if (pathStart != uri.end()) {
        result.path = std::string(pathStart, queryStart);
    }
    return result;

}

};

#include <iostream>

void printUri(const Uri &uri) {
    std::cout << uri.scheme << std::endl;
    std::cout << uri.host << std::endl;
    std::cout << uri.port << std::endl;
    std::cout << uri.path << std::endl;
    std::cout << uri.queryString << std::endl;
    std::cout << uri.fragment << std::endl;
}

int main() {
    Uri u0 = Uri::Parse("http://localhost:80/foo.html?&q=1:2:3");
    Uri u1 = Uri::Parse("https://localhost:80/foo.html?&q=1");
    Uri u2 = Uri::Parse("localhost/foo");
    Uri u3 = Uri::Parse("https://localhost/foo");
    Uri u4 = Uri::Parse("localhost:8080");
    Uri u5 = Uri::Parse("localhost?&foo=1");
    Uri u6 = Uri::Parse("localhost?&foo=1:2:3");

   printUri(u6);
   return 0;
}