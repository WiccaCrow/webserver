#include "CGI.hpp"
#include "Request.hpp"
#include "Client.hpp"

namespace HTTP {

CGI::CGI(void)
    : _isCompiled(false), _bodyPos(0) {
    for (size_t i = 0; i < 3; i++)
        _args[i] = NULL;
}

CGI::~CGI() { }

CGI::CGI(const CGI &other) {
    *this = other;
}

CGI &CGI::operator=(const CGI &other) {
    if (this != &other) {
        _execpath = other._execpath;
        _filepath = other._filepath;
        _args[0] = other._args[0];
        _args[1] = other._args[1];
        _args[2] = other._args[2];
        _isCompiled = other._isCompiled;
        _req = other._req;
        _res = other._res;
        // _ss.swap(other._ss);
        _headers = other._headers;
        _extraHeaders = other._extraHeaders;
    }
    return *this;
}

static void
restore_std(int in, int out) {
    if (in != -1 && dup2(in, fileno(stdin)) == -1) {
        Log.syserr() << "CGI::restore::in: " << std::endl;
    }
    if (out != -1 && dup2(out, fileno(stdout)) == -1) {
        Log.syserr() << "CGI::restore::out: " << std::endl;
    }
}

static void
close_pipe(int in, int out) {
    if (in != -1) {
        close(in);
    }
    if (out != -1) {
        close(out);
    }
}

static void
setValue(char *const env, const std::string &value) {
    char *ptr = strchr(env, '=');
    if (ptr == NULL) {
        return;
    }
    ptr[1] = '\0';
    strncat(env, value.c_str(), 1023 - strlen(env));
}

void
CGI::linkRequest(Request *req) {
    _req = req;
}

// This version passes all the env, including system
// Currently env pass with setEnv function.
void
CGI::setFullEnv(void) {
    setenv("PATH_INFO", "", 1);

    setenv("PATH_TRANSLATED", _req->getResolvedPath().c_str(), 1); // ?

    setenv("REMOTE_HOST", _req->getHeaderValue(HOST).c_str(), 1);
    setenv("REMOTE_ADDR", _req->getClient()->getIpAddr().c_str(), 1);
    setenv("REMOTE_USER", "", 1);
    setenv("REMOTE_IDENT", "", 1);

    setenv("AUTH_TYPE", _req->getLocation()->getAuthRef().isSet() ? "Basic" : "", 1);
    setenv("QUERY_STRING", _req->getQueryString().c_str(), 1);
    setenv("REQUEST_METHOD", _req->getMethod().c_str(), 1);
    setenv("SCRIPT_NAME", _req->getResolvedPath().c_str(), 1);
    setenv("CONTENT_LENGTH", sztos(_req->getBody().length()).c_str(), 1);
    setenv("CONTENT_TYPE", _req->getHeaderValue(CONTENT_TYPE).c_str(), 1);

    setenv("GATEWAY_INTERFACE", GATEWAY_INTERFACE, 1);
    setenv("SERVER_NAME",  _req->getUriRef().getAuthority().c_str(), 1);
    setenv("SERVER_SOFTWARE", SERVER_SOFTWARE, 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);

    // Current server block
    setenv("SERVER_PORT", sztos(_req->getServerBlock()->getPort()).c_str(), 1); // 80
    setenv("REDIRECT_STATUS", "200", 1);
}

void
CGI::setEnv(void) {

    // PATH_INFO
    setValue(env[0], "");
    
    // PATH_TRANSLATED
    setValue(env[1], _req->getResolvedPath()); // Definitely not like that
    
    // REMOTE_HOST
    setValue(env[2], _req->getHeaderValue(HOST)); // host, maybe should be without port
    
    // REMOTE_ADDR
    setValue(env[3], _req->getClient()->getIpAddr()); // ipv4 addr
    
    // REMOTE_USER
    setValue(env[4], "");
    
    // REMOTE_IDENT
    setValue(env[5], "");
    
    // AUTH_TYPE
    setValue(env[6], _req->getLocation()->getAuthRef().isSet() ? "Basic" : "");
    
    // QUERY_STRING
    setValue(env[7], _req->getQueryString());
    
    // REQUEST_METHOD
    setValue(env[8], _req->getMethod());
    
    // SCRIPT_NAME
    setValue(env[9], _req->getResolvedPath());
    
    // CONTENT_LENGTH
    setValue(env[10], sztos(_req->getBody().length()));
    
    // CONTENT_TYPE
    setValue(env[11], _req->getHeaderValue(CONTENT_TYPE));
    
    // GATEWAY_INTERFACE
    setValue(env[12], GATEWAY_INTERFACE);
    
    // SERVER_NAME
    setValue(env[13], _req->getUriRef().getAuthority()); // Not like that!
    
    // SERVER_SOFTWARE
    setValue(env[14], SERVER_SOFTWARE);
    
    // SERVER_PROTOCOL
    setValue(env[15], "HTTP/1.1");
    
    // SERVER_PORT
    setValue(env[16], sztos(_req->getServerBlock()->getPort()));

    // REDIRECT_STATUS
    setValue(env[17], "200");
}

void
CGI::setCompiled(bool value) {
    _isCompiled = value;
}

bool
CGI::isCompiled(void) {
    return _isCompiled;
}

void
CGI::setExecPath(const std::string path) {
    _execpath = path;
}

const std::string
CGI::getExecPath(void) const {
    return _execpath;
}

bool
CGI::setScriptPath(const std::string path) {
    _filepath = path;
    return true;
}

std::stringstream &
CGI::getResult(void) {
    return _ss;
}

void
CGI::clear(void) {
    _headers.clear();
    _extraHeaders.clear();
    _ss.str("");
    resetStream();
}

const std::list<ResponseHeader> &
CGI::getExtraHeaders(void) const {
    return _extraHeaders;
}

const std::list<ResponseHeader> &
CGI::getHeaders(void) const {
    return _headers;
}

const std::string 
CGI::getBody(void) const {
    return _ss.str().erase(0, _bodyPos);
}

size_t 
CGI::getBodyLength(void) {
    _ss.seekg(0, std::ios::end);
    size_t end = _ss.tellg();
    _ss.seekg(_bodyPos);
    return end - _bodyPos - 1;
}

int
CGI::exec() {
    if (_isCompiled && !isExecutableFile(_filepath)) {
        Log.error() << _filepath << " is not executable" << std::endl;
        return 0;
    }

    if (_isCompiled) {
        _args[0] = _filepath.c_str();
        _args[1] = "";
    } else {
        _args[0] = _execpath.c_str();
        _args[1] = _filepath.c_str();
    }

    int in[2]  = { -1 };
    int out[2] = { -1 };

    if (pipe(in) != 0) {
        Log.syserr() << "CGI::pipe::in: " << std::endl;
        return 0;
    }

    if (pipe(out) != 0) {
        Log.syserr() << "CGI::pipe::out: " << std::endl;
        close_pipe(in[0], in[1]);
        return 0;
    }

    int tmp[2] = { -1 };

    tmp[0] = dup(fileno(stdin));
    if (tmp[0] == -1) {
        Log.syserr() << "CGI::backup::in: " << std::endl;
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    tmp[1] = dup(fileno(stdout));
    if (tmp[1] == -1) {
        Log.syserr() << "CGI::backup::out: " << std::endl;
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    // Redirect for child process
    if (dup2(in[0], fileno(stdin)) == -1) {
        Log.syserr() << "CGI::redirect::in: " << std::endl;
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    if (dup2(out[1], fileno(stdout)) == -1) {
        Log.syserr() << "CGI::redirect::out: " << std::endl;
        restore_std(tmp[0], -1);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    int childPID = fork();
    if (childPID < 0) {
        Log.syserr() << "CGI::fork: " << std::endl;
        restore_std(tmp[0], tmp[1]);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;

    } else if (childPID == 0) {
        close_pipe(in[1], out[0]);
        if (execve(_args[0], const_cast<char *const *>(_args), env) == -1) {
            exit(125);
        }
    }

    if (!_req->getBody().empty()) {
        if (write(in[1], _req->getBody().c_str(), _req->getBody().length()) == -1) {
            Log.syserr() << "CGI::write: " << std::endl;
            restore_std(tmp[0], tmp[1]);
            close_pipe(tmp[0], tmp[1]);
            close_pipe(in[0], in[1]);
            close_pipe(out[0], out[1]);
            kill(childPID, SIGKILL);
            return 0;
        }
    }

    restore_std(tmp[0], tmp[1]);
    close_pipe(tmp[0], tmp[1]);
    close_pipe(in[0], in[1]);

    int status;
    waitpid(childPID, &status, 0);

    // Important to close it before reading
    close_pipe(-1, out[1]);

    if (WIFSIGNALED(status)) {
        Log.syserr() << "CGI::signaled:" << WTERMSIG(status) << std::endl;
        return 0;

    } else if (WIFSTOPPED(status)) {
        Log.syserr() << "CGI::stopped:" << WSTOPSIG(status) << std::endl;
        return 0;

    } else if (WIFEXITED(status)) {
        if (WEXITSTATUS(status)) {
            Log.syserr() << "CGI::exited:" << WEXITSTATUS(status) << std::endl;
            return 0;
        }

        int readBytes = 1;
        const int size = 4096;
        char buf[size];

        while (readBytes > 0) {
            readBytes = read(out[0], buf, size - 1);
            if (readBytes < 0) {
                Log.syserr() << "CGI::read" << std::endl;
                break ;
            }
            buf[readBytes] = 0;
            _ss << buf;
        }
    }
    close_pipe(out[0], -1);
    return 1;
}

void
CGI::resetStream(void) {
    _ss.clear();
    _ss.seekg(0, std::ios::beg);
}

void
CGI::parseHeaders(void) {

    for (std::string line; std::getline(_ss, line); ) {
        
        ResponseHeader header;
        
        rtrim(line, "\r\n");  
        if (header.parse(line)) {
            if (header.isValid()) {
                _headers.push_back(header);
            } else if (CGI::extraHeaderEnabled && 
                header.key.find(CGI::extraHeaderPrefix) == 0) {
                _extraHeaders.push_back(header);
            } else {
                _extraHeaders.clear();
                _headers.clear();
                return ;
            }
        } else {
            if (line.empty()) {
                if (!_headers.empty() || !_extraHeaders.empty()) {
                    _bodyPos = _ss.tellg();
                    return ;
                }
            }
            _extraHeaders.clear();
            _headers.clear();
            return ;
        }
    }
}

bool
CGI::isValidContentLength(void) {

    iter it = std::find(_headers.begin(), _headers.end(), ResponseHeader(CONTENT_LENGTH));

    if (it != _headers.end()) {
        long long length = strtoll(it->value.c_str(), NULL, 10);
        if (length < 0 || length > LONG_MAX) {
            Log.debug() << "Response::CGI:: ContentLength is invalid: " << length << std::endl;
            return false;
        }
        if (static_cast<size_t>(length) != getBodyLength()) {
            Log.debug() << "Response::CGI:: ContentLength mismatch" << std::endl;
            Log.debug() << "Response::CGI:: expected " << length << std::endl;
            Log.debug() << "Response::CGI:: got " << getBodyLength() << std::endl;
            return false;
        }
    }
    return true;
}

static char **
initEnv() {
    char **env = (char **)calloc(19, sizeof(char *));

    for (size_t i = 0; i < 18; i++) {
        env[i] = (char *)calloc(1024, sizeof(char));
    }

    strcpy(env[0], "PATH_INFO=");
    strcpy(env[1], "PATH_TRANSLATED=");
    strcpy(env[2], "REMOTE_HOST=");
    strcpy(env[3], "REMOTE_ADDR=");
    strcpy(env[4], "REMOTE_USER=");
    strcpy(env[5], "REMOTE_IDENT=");
    strcpy(env[6], "AUTH_TYPE=");
    strcpy(env[7], "QUERY_STRING=");
    strcpy(env[8], "REQUEST_METHOD=");
    strcpy(env[9], "SCRIPT_NAME=");
    strcpy(env[10], "CONTENT_LENGTH=");
    strcpy(env[11], "CONTENT_TYPE=");
    strcpy(env[12], "GATEWAY_INTERFACE=");
    strcpy(env[13], "SERVER_NAME=");
    strcpy(env[14], "SERVER_SOFTWARE=");
    strcpy(env[15], "SERVER_PROTOCOL=");
    strcpy(env[16], "SERVER_PORT=");
    strcpy(env[17], "REDIRECT_STATUS=");
    env[18] = NULL;

    return env;
}

char ** const CGI::env = initEnv();

const bool CGI::extraHeaderEnabled = true;
const std::string CGI::compiledExt = ".cgi";
const std::string CGI::extraHeaderPrefix = "X-CGI-";

}