#include "CGI.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Globals.hpp"
#include "Request.hpp"
#include "ValidHeaders.hpp"

static const char * pyargs[] = {
    static_cast<const char *>("/usr/bin/python"),
    static_cast<const char *>("./pages/site/printenv.py"),
    static_cast<const char *>(NULL)
};

// static const char * phpargs[] = {
//     static_cast<const char *>("/opt/homebrew/bin/php"),
//     static_cast<const char *>("./pages/site/printenv.php"),
//     static_cast<const char *>(NULL)
// };


void log_error(const std::string &prefix) {
    Log.error(prefix);
    Log.error("Errno: " + to_string(errno));
    Log.error("Description: " + std::string(strerror(errno)));
}

void restore_std(int in, int out) {
    if (in != -1 && dup2(in, fileno(stdin)) == -1) {
        log_error("CGI::restore::in: ");
    }
    if (out != -1 && dup2(out, fileno(stdout)) == -1) {
        log_error("CGI::restore::out: ");
    }
}

void close_pipe(int in, int out) {
    if (in != -1) {
        close(in);
    }
    if (out != -1) {
        close(out);
    }
}

void prepareEnv(HTTP::Request &req) {

    setenv("PATH_INFO", "", 1); 
    setenv("PATH_TRANSLATED", "", 1);
   
    // ?
    setenv("REMOTE_HOST", "", 1);  
    setenv("REMOTE_ADDR", "", 1);       // ip addr of the request sender
    setenv("REMOTE_USER", "", 1);
    setenv("REMOTE_IDENT", "", 1);

    // Request
    setenv("AUTH_TYPE", "Basic", 1);    // Basic, SSL, or null
    setenv("QUERY_STRING", req.getQueryString().c_str(), 1);      // scheme://authority/path?QUERY_STRING#fragment
    setenv("REQUEST_METHOD", req.getMethod().c_str(), 1);    // GET, POST ...
    setenv("SCRIPT_NAME", req.getScriptName().c_str(), 1);       // full script name ? hello.py
    setenv("CONTENT_LENGTH", to_string(req.getBody().length()).c_str(), 1);    // Number, 256, how much bytes should script read from stdin, -1 if not known
    // HeaderCode key = CONTENT_TYPE;
    setenv("CONTENT_TYPE", req.getHeaderValue(CONTENT_TYPE), 1);      // MIME-type, if there is, text/html

    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);   // CGI/revision
    setenv("SERVER_NAME", "localhost", 1);       // Server's hostname, DNS alias, or IP address as it appears in self-referencing URLs.
    setenv("SERVER_SOFTWARE", "webserv/1.0", 1); // our server name and version
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);    // HTTP/1.1 (scheme/revision)
    
    // Current server block
    setenv("SERVER_PORT", to_string(req.getServerBlock().getPort()).c_str(), 1);       // 80
    
    // Standart does not define these vars, but info appeared in different sources
    // setenv("DOCUMENT_ROOT", "", 1);
    // setenv("HTTP_COOKIE", "", 1);
    // setenv("HTTP_HOST", "", 1);
    // setenv("HTTP_ACCEPT", "", 1); // Comes from request

    // setenv("HTTP_FROM", "", 1);
    // setenv("HTTP_ACCEPT_CHARSET", "", 1);
    // setenv("HTTP_ACCEPT_ENCODING", "", 1);
    // setenv("HTTP_ACCEPT_LANGUAGE", "", 1);
    // setenv("HTTP_USER_AGENT", "", 1);
    // setenv("HTTP_REFERER", "", 1);
    // setenv("HTTP_FORWARDED", "", 1);
    // setenv("HTTP_PROXY_AUTHORIZATION", "", 1);
}


std::string CGI(HTTP::Request &req) {
    
    int in[2] = {-1};
    int out[2] = {-1};

    if (pipe(in) != 0) {
        log_error("CGI::pipe::in: ");
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    }

    if (pipe(out) != 0) {
        log_error("CGI::pipe::out: ");
        close_pipe(in[0], in[1]);
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    }

    int tmp[2] = {-1};
    tmp[0] = dup(fileno(stdin));
    if (tmp[0] == -1) {
        log_error("CGI::backup::in: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    }

    tmp[1] = dup(fileno(stdout));
    if (tmp[1] == -1) {
        log_error("CGI::backup::out: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    }

    // Redirect for child process
    if (dup2(in[0], fileno(stdin)) == -1) {
        log_error("CGI::redirect::in: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    }
    
    if (dup2(out[1], fileno(stdout)) == -1) {
        log_error("CGI::redirect::out: ");
        restore_std(tmp[0], -1);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    }
    
    prepareEnv(req);
    int childPID = fork();
    if (childPID < 0) {
        log_error("CGI::fork: ");
        restore_std(tmp[0], tmp[1]);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        // return some message ?
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    } else if (childPID == 0) {
        close_pipe(in[1], out[0]);
        // dup2(out[1], fileno(stderr));
        char buf[1000];
        realpath(pyargs[1], buf);
        pyargs[1] = buf;
        if (execv(pyargs[0], (char * const *)pyargs) == -1) {
            exit(3);
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
    //         return "";
    //     }
    // }

    restore_std(tmp[0], tmp[1]);
    close_pipe(tmp[0], tmp[1]);
    close_pipe(in[0], in[1]);

    int status;
    waitpid(childPID, &status, 0);

    // Important to close it before reading
    close_pipe(-1, out[1]);
    
    if (WEXITSTATUS(status) == 3) {
        log_error("CGI::execv: ");
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        return "";
    }

        std::string res = "";
    if (WIFEXITED(status)) {
        int r = 1;
        const int size = 300;
        char buf[size];

        while (r > 0) {
            r = read(out[0], buf, size - 1);
            if (r < 0) {
                std::cout << "read err" << std::endl;
                break ;
            }
            buf[r] = 0;
            res += buf;
        }
        // Send response to the client in body
        std::cout << res << std::endl;
    }
    // Add other macros for handling unexpected termination of child process.

    close_pipe(out[0], -1);

    return res;
}
