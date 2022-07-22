#include "CGI.hpp"

#include "Client.hpp"
#include "Request.hpp"
#include "Server.hpp"

namespace HTTP {

static const std::size_t envCount = 18;
static const std::size_t envVarLen = 1024;

CGI::CGI(void)
    : _compiled(false), _env(NULL), _childPID(-1) {}

CGI::~CGI(void) {
    if (_env != NULL) {
        for (std::size_t i = 0; i < envCount; i++) {
            delete[] _env[i];
        }
        delete[] _env;
    }
}

CGI::CGI(const CGI &other) {
    *this = other;
}

CGI &CGI::operator=(const CGI &other) {
    if (this != &other) {
        _execpath = other._execpath;
        _filepath = other._filepath;
        _compiled = other._compiled;
        _env = other._env;
        _childPID = other._childPID;
    }
    return *this;
}

int CGI::getPID(void) const {
    return _childPID;
}

void CGI::setPID(int pid) {
    _childPID = pid;
}

static void
setValue(char *const env, const std::string value) {
    char *ptr = strchr(env, '=');
    if (ptr == NULL) {
        return;
    }
    ptr[1] = '\0';
    strncat(env, value.c_str(), envVarLen - 1 - strlen(env));
}

bool CGI::setEnv(Request *req) {

    if (!initEnv()) {
        return false;
    }

    // PATH_INFO
    setValue(_env[0], req->getPathInfo());
    // setValue(_env[0], req->getUriRef()._path);

    // PATH_TRANSLATED
    setValue(_env[1], req->getResolvedPath()); // Definitely not like that!

    const std::string &host = req->getClient()->getDomainName();
    const std::string &addr = req->getClient()->getClientIO()->getAddr();

    // REMOTE_HOST
    setValue(_env[2], host.empty() ? addr : host);

    // REMOTE_ADDR
    setValue(_env[3], addr);

    // REMOTE_USER
    setValue(_env[4], req->getRemoteUser());

    // REMOTE_IDENT
    setValue(_env[5], "");

    // AUTH_TYPE
    setValue(_env[6], req->getLocation()->getAuthRef().getType());

    // QUERY_STRING
    setValue(_env[7], req->getUriRef()._query);

    // REQUEST_METHOD
    setValue(_env[8], req->getMethod());

    // SCRIPT_NAME
    setValue(_env[9], req->getUriRef()._path); // PATH_INFO should be excluded

    // CONTENT_LENGTH
    Log.debug() << "CGI: body length: " << req->getBody().length() << Log.endl; 
    setValue(_env[10], sztos(req->getBody().length()));

    // CONTENT_TYPE
    setValue(_env[11], req->headers[CONTENT_TYPE].value) ;

    // GATEWAY_INTERFACE
    setValue(_env[12], GATEWAY_INTERFACE);

    // SERVER_NAME
    setValue(_env[13], req->getClient()->getServerIO()->getAddr());

    // SERVER_SOFTWARE
    setValue(_env[14], SERVER_SOFTWARE);

    // SERVER_PROTOCOL
    setValue(_env[15], SERVER_PROTOCOL);

    // SERVER_PORT
    setValue(_env[16], sztos(req->getClient()->getServerIO()->getPort()));

    // REDIRECT_STATUS
    setValue(_env[17], "200");

    return true;
}

void CGI::compiled(bool value) {
    _compiled = value;
}

bool CGI::compiled(void) {
    return _compiled;
}

void CGI::setExecPath(const std::string path) {
    _execpath = path;
}

const std::string
CGI::getExecPath(void) const {
    return _execpath;
}

void
CGI::setScriptPath(const std::string &path) {
    _filepath = path;
}

int
CGI::exec(Request *req) {

    if (compiled() && !isExecutableFile(_filepath)) {
        Log.error() << _filepath << " is not executable" << Log.endl;
        return 0;
    
    } else if (!compiled() && !isExecutableFile(_execpath)) {
        Log.error() << _execpath << " is not executable" << Log.endl;
        return 0;
    }

    if (!setEnv(req)) {
        Log.error() << "CGI::setEnv " << Log.endl;
        return 0;
    }

    const char *args[3] = {0};
    if (compiled()) {
        args[0] = _filepath.c_str();
        args[1] = "";
    } else {
        args[0] = _execpath.c_str();
        args[1] = _filepath.c_str();
    }

    IO in;
    IO out;

    if (in.pipe() != 0) {
        Log.syserr() << "CGI::pipe::in: " << Log.endl;
        return 0;
    }

    if (out.pipe() != 0) {
        Log.syserr() << "CGI::pipe::out: " << Log.endl;
        close(in.rdFd());
        close(in.wrFd());
        return 0;
    }

    Client *client = req->getClient();
    IO *io = client->getGatewayIO();
    io->rdFd(out.rdFd());
    io->wrFd(in.wrFd());

    int childPID = fork();
    if (childPID < 0) {
        Log.syserr() << "CGI::exec: fork failed" << Log.endl;
        return 0;

    } else if (childPID == 0) {

        const std::string dir = getDirectory(_filepath);
        if (!dir.empty()) {
            chdir(dir.c_str());
        }

        close(in.wrFd());
        close(out.rdFd());

        if (dup2(in.rdFd(), fileno(stdin)) < 0) {
            Log.syserr() << "CGI::exec: dup2 failed for stdin" << Log.endl;
            exit(123);
        }
    
        if (dup2(out.wrFd(), fileno(stdout)) < 0) {
            Log.syserr() << "CGI::exec: dup2 failed for stdout" << Log.endl;
            exit(124);
        }
    
        if (execve(args[0], const_cast<char * const *>(args), _env) == -1) {
            Log.syserr() << "CGI::exec: execve failed for " << args[0] << Log.endl;
            exit(125);
        }
    }

    close(in.rdFd());
    close(out.wrFd());

    setPID(childPID);

    req->isCGI(true);
    
    if (!req->getBody().empty()) {
        g_server->link(io->wrFd(), client);
    } else {
        close(io->wrFd());
    }
    g_server->link(io->rdFd(), client);
    return 1;
}

bool CGI::initEnv(void) {

    _env = new char *[envCount + 1];
    if (_env == NULL) {
        return false;
    }

    for (std::size_t i = 0; i < envCount; i++) {
        _env[i] = NULL;
    }

    for (std::size_t i = 0; i < envCount; i++) {
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

} // namespace HTTP