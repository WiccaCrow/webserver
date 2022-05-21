#include "CGI.hpp"

#include "Logger.hpp"

//Different fields (e.g. name value pairs are separated by a ampersand (&).
//Name/value pair assignments are denoted by an equals sign (=). The format is name=value.
//Blank spaces must be denoted by a plus sign +.
//Some special characters will be replaced by a percent sign (2 digit hexadecimal (ASCII Value) code. For example if you need to input an actual &

// Should be parsed and passed from server
static const std::string body = "body\n";
static const std::string header = "Content-Length=" + to_string(body.length());
static const char *      env[] = {header.c_str(), "VAR1=mhufflep", 0, 0};
static const char *      args[] = {"/usr/bin/python3.8", "/var/www/scripts/test.py", "args", 0, 0};

// rename function to CGI
int main() {
    int inPipe[2] = {-1};
    int outPipe[2] = {-1};

    if (pipe(inPipe) != 0 || pipe(outPipe) != 0) {
        Log.error("CGI: Cannot create pipes");
        return 1;
    }

    int stdinCopy = dup(fileno(stdin));
    int stdoutCopy = dup(fileno(stdout));

    // Redirect for child process
    if ((dup2(outPipe[1], fileno(stdout)) == -1) || (dup2(inPipe[0], fileno(stdin)) == -1)) {
        Log.error("CGI: dup2 function failed (main proc)");
        return 1;
    }

    int childPID = fork();

    if (childPID < 0) {
        Log.error("CGI: fork failed");
        // Restore fds and close pipes
        return 1;
    }

    if (childPID == 0) {
        execvpe(args[0], (char *const *)args, (char *const *)env);
        exit(1);
    }

    close(inPipe[0]);
    close(outPipe[1]);

    // Check for errors
    dup2(stdinCopy, fileno(stdin));
    dup2(stdoutCopy, fileno(stdout));

    close(stdinCopy);
    close(stdoutCopy);

    // Safe maybe
    write(inPipe[1], body.c_str(), body.length());

    while (1) {
        char buf[100000];
        // Definitely should be safe
        // Timeout for response ???
        int n = read(outPipe[0], buf, 100000);
        if (n > 0) {
            // Send response to the client in body
        }

        int status;
        if (waitpid(childPID, &status, WNOHANG) > 0)
            break;
    }

    // Important
    close(inPipe[1]);
    close(outPipe[0]);

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