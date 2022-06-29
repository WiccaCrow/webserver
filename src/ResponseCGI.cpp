#include "ResponseCGI.hpp"

namespace HTTP {

const bool ResponseCGI::extraHeadersEnabled = true;
const std::string ResponseCGI::extraHeaderPrefix = "X-CGI-";

ResponseCGI::ResponseCGI(void)
    : _client(NULL)
    , _bodyLength(0)
    , _isFormed(false)
    , _status(OK) {}

ResponseCGI::~ResponseCGI(void) {}

void
ResponseCGI::parse(std::string &line) {

    ResponseHeader header;
    
    rtrim(line, "\r\n");  
    if (header.parse(line)) {
        if (header.isValid()) {
            // header.read();
            _headers.push_back(header);
        } else if (extraHeadersEnabled && 
            header.key.find(extraHeaderPrefix) == 0) {
            _extraHeaders.push_back(header);
        } else {
            _extraHeaders.clear();
            _headers.clear();
            return ;
        }
    } else {
        if (line.empty()) {
            if (!_headers.empty() || !_extraHeaders.empty()) {
                // _bodyPos = _ss.tellg();
                return ;
            }
        }
        _extraHeaders.clear();
        _headers.clear();
        return ;
    }
}

}