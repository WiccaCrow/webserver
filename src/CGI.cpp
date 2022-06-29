#include "CGI.hpp"
#include "Request.hpp"
#include "Client.hpp"
#include "ClientCGI.hpp"
#include "Server.hpp"

namespace HTTP {

static const size_t envCount = 18;
static const size_t envVarLen = 1024;

CGI::CGI(void)
    : _compiled(false)
    , _env(NULL) {}

CGI::~CGI(void) {
    for (size_t i = 0; i < envCount; i++) {
        delete[] _env[i];
    }
    delete[] _env;
}

CGI::CGI(const CGI &other) {
    *this = other;
}

CGI &
CGI::operator=(const CGI &other) {
    if (this != &other) {
        _execpath = other._execpath;
        _filepath = other._filepath;
        _compiled = other._compiled;
        _req = other._req;
        _res = other._res;
        _env = other._env;
    }
    return *this;
}

static void
restore_std(int in, int out) {
    if (in != -1 && dup2(in, fileno(stdin)) == -1) {
        Log.syserr() << "CGI::restore::in: " << Log.endl;
    }
    if (out != -1 && dup2(out, fileno(stdout)) == -1) {
        Log.syserr() << "CGI::restore::out: " << Log.endl;
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
        return ;
    }
    ptr[1] = '\0';
    strncat(env, value.c_str(), envVarLen - 1 - strlen(env));
}

void
CGI::link(Request *req, Response *res) {
    _req = req;
    _res = res;
}


// This version passes all the env, including system
// Currently env pass with setEnv function.
bool
CGI::setFullEnv(void) {

    if (!initEnv()) {
        return false;
    }

    setenv("PATH_INFO", "", 1);
    setenv("PATH_TRANSLATED", _req->getResolvedPath().c_str(), 1); // ?

    setenv("REMOTE_HOST", _req->getHeaderValue(HOST).c_str(), 1);
    setenv("REMOTE_ADDR", _req->getClient()->getIpAddr().c_str(), 1);
    setenv("REMOTE_USER", "", 1);
    setenv("REMOTE_IDENT", "", 1);

    setenv("AUTH_TYPE", _req->getLocation()->getAuthRef().getType().c_str(), 1);
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

bool
CGI::setEnv(void) {

    if (!initEnv()) {
        return false;
    }
    // PATH_INFO
    setValue(_env[0], "");

    // PATH_TRANSLATED
    setValue(_env[1], _req->getResolvedPath()); // Definitely not like that
    
    // REMOTE_HOST
    setValue(_env[2], _req->getHeaderValue(HOST)); // host, maybe should be without port
    
    // REMOTE_ADDR
    setValue(_env[3], _req->getClient()->getIpAddr()); // ipv4 addr
    
    // REMOTE_USER
    setValue(_env[4], "");
    
    // REMOTE_IDENT
    setValue(_env[5], "");
    
    // AUTH_TYPE
    setValue(_env[6], _req->getLocation()->getAuthRef().getType());
    
    // QUERY_STRING
    setValue(_env[7], _req->getQueryString());
    
    // REQUEST_METHOD
    setValue(_env[8], _req->getMethod());
    
    // SCRIPT_NAME
    setValue(_env[9], _req->getResolvedPath());
    
    // CONTENT_LENGTH
    setValue(_env[10], sztos(_req->getBody().length()));
    
    // CONTENT_TYPE
    setValue(_env[11], _req->getHeaderValue(CONTENT_TYPE));
    
    // GATEWAY_INTERFACE
    setValue(_env[12], GATEWAY_INTERFACE);
    
    // SERVER_NAME
    setValue(_env[13], _req->getUriRef().getAuthority()); // Not like that!
    
    // SERVER_SOFTWARE
    setValue(_env[14], SERVER_SOFTWARE);
    
    // SERVER_PROTOCOL
    setValue(_env[15], "HTTP/1.1");
    
    // SERVER_PORT
    setValue(_env[16], sztos(_req->getServerBlock()->getPort()));

    // REDIRECT_STATUS
    setValue(_env[17], "200");
}

void
CGI::compiled(bool value) {
    _compiled = value;
}

bool
CGI::compiled(void) {
    return _compiled;
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

int
CGI::exec() {
    if (compiled() && !isExecutableFile(_filepath) || !isExecutableFile(_execpath)) {
        Log.error() << _filepath << " is not executable" << Log.endl;
        return 0;
    }

    int in[2]  = { -1 };
    int out[2] = { -1 };

    if (pipe(in) != 0) {
        Log.syserr() << "CGI::pipe::in: " << Log.endl;
        return 0;
    }

    if (pipe(out) != 0) {
        Log.syserr() << "CGI::pipe::out: " << Log.endl;
        close_pipe(in[0], in[1]);
        return 0;
    }

    int tmp[2] = { -1 };

    tmp[0] = dup(fileno(stdin));
    if (tmp[0] == -1) {
        Log.syserr() << "CGI::backup::in: " << Log.endl;
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    tmp[1] = dup(fileno(stdout));
    if (tmp[1] == -1) {
        Log.syserr() << "CGI::backup::out: " << Log.endl;
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    // Redirect for child process
    if (dup2(in[0], fileno(stdin)) == -1) {
        Log.syserr() << "CGI::redirect::in: " << Log.endl;
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    if (dup2(out[1], fileno(stdout)) == -1) {
        Log.syserr() << "CGI::redirect::out: " << Log.endl;
        restore_std(tmp[0], -1);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    const char *args[3] = { 0 };
    if (compiled()) {
        args[0] = _filepath.c_str();
        args[1] = "";
    } else {
        args[0] = _execpath.c_str();
        args[1] = _filepath.c_str();
    }

    int childPID = fork();
    if (childPID < 0) {
        Log.syserr() << "CGI::fork: " << Log.endl;
        restore_std(tmp[0], tmp[1]);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;

    } else if (childPID == 0) {
        close_pipe(in[1], out[0]);
        if (execve(args[0], const_cast<char * const *>(args), _env) == -1) {
            exit(125);
        }
    }

    restore_std(tmp[0], tmp[1]);
    close_pipe(tmp[0], tmp[1]);
    close_pipe(in[0], -1);
    close_pipe(-1, out[1]);
    // Important to close it before reading

    if (!_req->getBody().empty()) {
        _res->getClient()->setWriteFd(in[1]);
    }

    ClientCGI *client = new ClientCGI();
    client->setReadFd(out[0]);
    // Place twice in pollfds ? If not, then pollout will never be checked
    client->setWriteFd(_res->getClient()->getReadFd());
    g_server->addSocket((struct pollfd) { out[0], POLLIN, 0 }, client);

    return 1;
}


bool
CGI::initEnv(void) {

    _env = new char *[envCount + 1];
    if (_env == NULL) {
        return false;
    }

    for (size_t i = 0; i < envCount; i++) {
        _env[i] = new char[envVarLen];
        if (_env[i] == NULL) {
            return false;
        }
    }
    strcpy(_env[0], "PATH_INFO=");
    strcpy(_env[1], "PATH_TRANSLATED=");
    strcpy(_env[2], "REMOTE_HOST=");
    strcpy(_env[3], "REMOTE_ADDR=");
    strcpy(_env[4], "REMOTE_USER=");
    strcpy(_env[5], "REMOTE_IDENT=");
    strcpy(_env[6], "AUTH_TYPE=");
    strcpy(_env[7], "QUERY_STRING=");
    strcpy(_env[8], "REQUEST_METHOD=");
    strcpy(_env[9], "SCRIPT_NAME=");
    strcpy(_env[10], "CONTENT_LENGTH=");
    strcpy(_env[11], "CONTENT_TYPE=");
    strcpy(_env[12], "GATEWAY_INTERFACE=");
    strcpy(_env[13], "SERVER_NAME=");
    strcpy(_env[14], "SERVER_SOFTWARE=");
    strcpy(_env[15], "SERVER_PROTOCOL=");
    strcpy(_env[16], "SERVER_PORT=");
    strcpy(_env[17], "REDIRECT_STATUS=");
    _env[18] = NULL;

    return true;
}

const std::string CGI::compiledExt = ".cgi";

}