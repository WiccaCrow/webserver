#include "CGI.hpp"
#include "Request.hpp"

namespace HTTP {

CGI::CGI(void)
    : _isCompiled(false) {
    for (size_t i = 0; i < 3; i++)
        _args[i] = NULL;
}

CGI::~CGI() { }

static void
log_error(const std::string &location) {
    Log.error(location);
    Log.error("Errno: " + to_string(errno));
    Log.error("Description: " + std::string(strerror(errno)));
}

static void
restore_std(int in, int out) {
    if (in != -1 && dup2(in, fileno(stdin)) == -1) {
        log_error("CGI::restore::in: ");
    }
    if (out != -1 && dup2(out, fileno(stdout)) == -1) {
        log_error("CGI::restore::out: ");
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
setValue(char *const env, std::string value) {
    char *ptr = strchr(env, '=');
    if (ptr == NULL) {
        return;
    }
    ptr[1] = '\0';
    strncat(env, value.c_str(), 1023 - strlen(env));
}

void
CGI::setFullEnv(Request &req) {
    setenv("PATH_INFO", "", 1);

    std::string s = req.getLocationPtr()->getRootRef() + req.getPath();
    setenv("PATH_TRANSLATED", s.c_str(), 1);

    setenv("REMOTE_HOST", "", 1);
    setenv("REMOTE_ADDR", "", 1);
    setenv("REMOTE_USER", "", 1);
    setenv("REMOTE_IDENT", "", 1);

    setenv("AUTH_TYPE", "Basic", 1);
    setenv("QUERY_STRING", req.getQueryString().c_str(), 1);
    setenv("REQUEST_METHOD", req.getMethod().c_str(), 1);
    setenv("SCRIPT_NAME", req.getScriptName().c_str(), 1);
    setenv("CONTENT_LENGTH", to_string(req.getBody().length()).c_str(), 1);
    setenv("CONTENT_TYPE", req.getHeaderValue(CONTENT_TYPE), 1);

    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
    setenv("SERVER_NAME", "localhost", 1);
    setenv("SERVER_SOFTWARE", "webserv/1.0", 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);

    // Current server block
    setenv("SERVER_PORT", to_string(req.getServerBlock()->getPort()).c_str(), 1); // 80
    setenv("REDIRECT_STATUS", "200", 1);
}

void
CGI::setEnv(Request &req) {
    setValue(env[0], "");
    setValue(env[1], req.getLocationPtr()->getRootRef() + req.getPath());
    setValue(env[2], "");
    setValue(env[3], "");
    setValue(env[4], "");
    setValue(env[5], "");
    setValue(env[6], "Basic");
    setValue(env[7], req.getQueryString());
    setValue(env[8], req.getMethod());
    setValue(env[9], req.getScriptName());
    setValue(env[10], to_string(req.getBody().length()));
    setValue(env[11], req.getHeaderValue(CONTENT_TYPE));
    setValue(env[12], "CGI/1.1");
    setValue(env[13], "localhost");
    setValue(env[14], "webserv/1.0");
    setValue(env[15], "HTTP/1.1");
    setValue(env[16], to_string(req.getServerBlock()->getPort()));
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

const std::string
CGI::getResult(void) const {
    return _res;
}

void
CGI::reset(void) {
    _res = "";
}

int
CGI::exec() {
    if (_isCompiled && !isExecutableFile(_filepath)) {
        Log.error(_filepath + " is not executable");
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
        log_error("CGI::pipe::in: ");
        return 0;
    }

    if (pipe(out) != 0) {
        log_error("CGI::pipe::out: ");
        close_pipe(in[0], in[1]);
        return 0;
    }

    int tmp[2] = { -1 };

    tmp[0] = dup(fileno(stdin));
    if (tmp[0] == -1) {
        log_error("CGI::backup::in: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    tmp[1] = dup(fileno(stdout));
    if (tmp[1] == -1) {
        log_error("CGI::backup::out: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    // Redirect for child process
    if (dup2(in[0], fileno(stdin)) == -1) {
        log_error("CGI::redirect::in: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    if (dup2(out[1], fileno(stdout)) == -1) {
        log_error("CGI::redirect::out: ");
        restore_std(tmp[0], -1);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 0;
    }

    int childPID = fork();
    if (childPID < 0) {
        log_error("CGI::fork: ");
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

    // if (req.getBody() != "") {
    //     if (write(in[1], req.getBody().c_str(), req.getBody().length()) == -1) {
    //         log_error("CGI::write: ");
    //         restore_std(tmp[0], tmp[1]);
    //         close_pipe(tmp[0], tmp[1]);
    //         close_pipe(in[0], in[1]);
    //         close_pipe(out[0], out[1]);
    //         kill(childPID, SIGKILL);
    //         req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
    //         return 0;
    //     }
    // }

    restore_std(tmp[0], tmp[1]);
    close_pipe(tmp[0], tmp[1]);
    close_pipe(in[0], in[1]);

    int status;
    waitpid(childPID, &status, 0);

    // Important to close it before reading
    close_pipe(-1, out[1]);

    if (WIFSIGNALED(status)) {
        log_error("CGI::signaled:" + to_string(WTERMSIG(status)));
        return 0;
    } else if (WIFSTOPPED(status)) {
        log_error("CGI::stopped:" + to_string(WSTOPSIG(status)));
        return 0;
    } else if (WIFEXITED(status)) {

        if (WEXITSTATUS(status)) {
            log_error("CGI::exited:" + to_string(WEXITSTATUS(status)));
            return 0;
        }

        int       readBytes = 1;
        const int size      = 300;

        char buf[size];
        while (readBytes > 0) {
            readBytes = read(out[0], buf, size - 1);
            if (readBytes < 0) {
                log_error("CGI::read");
                return 1;
            }
            buf[readBytes] = 0;
            _res += buf;
        }
        // Send response to the client in body
    }
    close_pipe(out[0], -1);
    return 1;
}

char **
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

char **CGI::env = initEnv();

const std::string CGI::compiledExt = ".cgi";

}