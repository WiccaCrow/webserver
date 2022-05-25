#include "CGI.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Globals.hpp"

static char * const pyargs[] = {
    "/usr/bin/python",
    "/Users/mhufflep/Desktop/webserver/pages/site/printenv.py",
};

static char * const phpargs[] = {
    "/opt/homebrew/bin/php",
    "/Users/mhufflep/Desktop/webserver/pages/site/printenv.php",
};


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

void prepareEnv() {
    
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1); // CGI/revision
    
    setenv("PATH_INFO", "", 1); 
    setenv("PATH_TRANSLATED", "", 1);
   
    // ?
    setenv("REMOTE_HOST", "", 1);  
    setenv("REMOTE_ADDR", "", 1);       // ip addr of the request sender
    setenv("REMOTE_USER", "", 1);
    setenv("REMOTE_IDENT", "", 1);

    // Request
    setenv("AUTH_TYPE", "Basic", 1);    // Basic, SSL, or null
    setenv("QUERY_STRING", "", 1);      // scheme://authority/path?QUERY_STRING#fragment
    setenv("REQUEST_METHOD", "", 1);    // GET, POST ...
    setenv("SCRIPT_NAME", "", 1);       // full script name ? hello.py
    setenv("CONTENT_TYPE", "", 1);      // MIME-type, if there is, text/html
    setenv("CONTENT_LENGTH", "", 1);    // Number, 256, how much bytes should script read from stdin, -1 if not known
    
    setenv("SERVER_NAME", "", 1);                // Server's hostname, DNS alias, or IP address as it appears in self-referencing URLs.
    setenv("SERVER_SOFTWARE", "webserv/1.0", 1); // our server name and version
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);    // HTTP/1.1 (proto/revision)
    
    // Current server block
    setenv("SERVER_PORT", "", 1);       // 80
    
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


int main() {
    
    int in[2] = {-1};
    int out[2] = {-1};

    if (pipe(in) != 0) {
        log_error("CGI::pipe::in: ");
        return 1;
    }

    if (pipe(out) != 0) {
        log_error("CGI::pipe::out: ");
        close_pipe(in[0], in[1]);
        return 1;
    }

    int tmp[2] = {-1};
    tmp[0] = dup(fileno(stdin));
    if (tmp[0] == -1) {
        log_error("CGI::backup::in: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 1;
    }

    tmp[1] = dup(fileno(stdout));
    if (tmp[1] == -1) {
        log_error("CGI::backup::out: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 1;
    }

    // Redirect for child process
    if (dup2(in[0], fileno(stdin)) == -1) {
        log_error("CGI::redirect::in: ");
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 1;
    }
    
    if (dup2(out[1], fileno(stdout)) == -1) {
        log_error("CGI::redirect::out: ");
        restore_std(tmp[0], -1);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        return 1;
    }
    
    
    int childPID = fork();
    if (childPID < 0) {
        log_error("CGI::fork: ");
        restore_std(tmp[0], tmp[1]);
        close_pipe(tmp[0], tmp[1]);
        close_pipe(in[0], in[1]);
        close_pipe(out[0], out[1]);
        // return some message ?
        return 1;
    } else if (childPID == 0) {
        prepareEnv();
        close_pipe(in[1], out[0]);
        dup2(fileno(stdout), fileno(stderr));
        if (execv(pyargs[0], (char * const *)pyargs) == -1) {
            log_error("CGI::execv: ");
            exit(1);
        }
    }

    // write(in[1], body.c_str(), body.length());
    // needs to be protected because internal 
    // buffer could overflow with large body

    restore_std(tmp[0], tmp[1]);
    close_pipe(tmp[0], tmp[1]);
    close_pipe(in[0], in[1]);

    int status;
    waitpid(childPID, &status, 0);

    // Important to close it before reading
    close_pipe(-1, out[1]);
    
    if (WIFEXITED(status)) {
        std::string res = "";
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

    return 0;
}

// Web server should send body to stdin of the program (if there is any)
// and send some of headers and parameters as an env


// GATEWAY_INTERFACE The revision of the Common Gateway Interface that the server uses.
// SERVER_NAME The server's hostname or IP address.
// SERVER_SOFTWARE The name and version of the server software that is answering the client request.
// SERVER_PROTOCOL The name and revision of the information protocol the request came in with.
// SERVER_PORT The port number of the host on which the server is running.
// REQUEST_METHOD The method with which the information request was issued.
// PATH_INFO Extra path information passed to a CGI program.
// PATH_TRANSLATED The translated version of the path given by the variable PATH_INFO.
// SCRIPT_NAME The virtual path (e.g., /cgi-bin/program.pl) of the script being executed.
// DOCUMENT_ROOT The directory from which Web documents are served.
// QUERY_STRING The query information passed to the program. It is appended to the URL with a "?".
// REMOTE_HOST The remote hostname of the user making the request.
// REMOTE_ADDR The remote IP address of the user making the request.
// AUTH_TYPE The authentication method used to validate a user.
// REMOTE_USER The authenticated name of the user.
// REMOTE_IDENT The user making the request. This variable will only be set if NCSA IdentityCheck flag is enabled, and the client machine supports the RFC 931 identification scheme (ident daemon).
// CONTENT_TYPE The MIME type of the query data, such as "text/html".
// CONTENT_LENGTH The length of the data (in bytes or the number of characters) passed to the CGI program through standard input.
// HTTP_FROM The email address of the user making the request. Most browsers do not support this variable.
// HTTP_ACCEPT A list of the MIME types that the client can accept.
// HTTP_USER_AGENT The browser the client is using to issue the request.
// HTTP_REFERER The URL of the document that the client points to before accessing the CGI program.