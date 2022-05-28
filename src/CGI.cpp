#include "CGI.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Globals.hpp"
#include "Request.hpp"
#include "ValidHeaders.hpp"

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

void prepareEnv(HTTP::Request &req, const char *env[]) {

    std::string path_info = "PATH_INFO="; 
    std::string path_translated = "PATH_TRANSLATED=" + req.getLocationPtr()->getRootRef() + req.getPath(); 
    std::string remote_host = "REMOTE_HOST=";
    std::string remote_addr = "REMOTE_ADDR=";
    std::string remote_user = "REMOTE_USER=";
    std::string remote_ident = "REMOTE_IDENT=";
    std::string auth_type = "AUTH_TYPE=Basic";
    std::string query_string = "QUERY_STRING=" + req.getQueryString();
    std::string request_method = "REQUEST_METHOD=" + req.getMethod();
    std::string script_name = "SCRIPT_NAME=" + req.getScriptName();
    std::string content_length = "CONTENT_LENGTH=" + to_string(req.getBody().length());
    std::string content_type = static_cast<std::string>("CONTENT_TYPE=") + req.getHeaderValue(CONTENT_TYPE);
    std::string gateway_interface = "GATEWAY_INTERFACE=CGI/1.1";
    std::string server_name = "SERVER_NAME=localhost";
    std::string server_software = "SERVER_SOFTWARE=webserv/1.0";
    std::string server_protocol = "SERVER_PROTOCOL=HTTP/1.1";
    std::string server_port = "SERVER_PORT=" + to_string(req.getServerBlock().getPort());
    std::string redirect_status = "REDIRECT_STATUS=200";

     env[0] = path_info.c_str();
     env[1] = path_translated.c_str();
     env[2] = remote_host.c_str();
     env[3] = remote_addr.c_str();
     env[4] = remote_user.c_str();
     env[5] = remote_ident.c_str();
     env[6] = auth_type.c_str();
     env[7] = query_string.c_str();
     env[8] = request_method.c_str();
     env[9] = script_name.c_str();
    env[10] = content_length.c_str();
    env[11] = content_type.c_str();
    env[12] = gateway_interface.c_str();
    env[13] = server_name.c_str();
    env[14] = server_software.c_str();
    env[15] = server_protocol.c_str();
    env[16] = server_port.c_str();
    env[17] = redirect_status.c_str();
}


std::string CGI(HTTP::Request &req, std::map<std::string, std::string>::const_iterator it) {

    std::string s = req.getLocationPtr()->getRootRef() + req.getPath();
    std::string z = it->first == ".cgi" ? s : it->second;
    const char * args[] = {
        z.c_str(),
        s.c_str(),
        NULL
    };

    Log.debug(args[0]);
    Log.debug(args[1]);

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

        // Catch stderr from cgi
        // dup2(out[1], fileno(stderr));

        const char *env[20] = {0};
        prepareEnv(req, env);
        if (execve(args[0], (char * const *)args, const_cast<char * const *>(env)) == -1) {
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
