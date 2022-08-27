// g++ -Wall -Wextra -Werror puttest.cpp -o puttest
// ./puttest h:127.0.0.1 p:8080 s:123  m:POST u:/post/
// ./puttest h:127.0.0.1 p:8080 s:123M m:POST u:/post/
// ./puttest h:127.0.0.1 p:8080 s:123G m:POST u:/post/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>

#define MAXBODY 20000000000

long    setMsgBodySize(char *size);
long    checkSizeFormat(char *size);

std::string    setIpFormat(char *IPv4);
std::string    setkPortFormat(char *port);
std::string    setMethod(char *method);

int     connectToServer(const char *IPv4, const char *port);
void*   get_in_addr(struct sockaddr *sa);
void    sendMSG(int sockfd, long stringsize, 
                std::string method, std::string urn, 
                std::string hostport); 

int main(int ac, char *av[]) {
    if (ac > 6) {
        std::cerr << "Error. too many arguments (max 5) \n\t(example h:127.0.0.1 p:8080 s:123M m:POST u:/post/)" << std::endl;
        return 1;
    }
    long bodySize = 0;
    std::string method("PUT");
    std::string port("8080");
    std::string urn("/");
    std::string ip("127.0.0.1");

    for (int i = 1; i < ac; ++i) {
        if (strlen(av[i]) < 3 || av[i][1] != ':') {
            std::cerr << "Error. h:IPv4 p:port s:size m:method(PUT or POST) u:urn \n\t(example h:127.0.0.1 p:8080 s:123M m:POST u:/post/)" << std::endl;
            return 1;
        }
        if (av[i][0] == 'h') {
            ip = setIpFormat(av[i] + 2);
        } else if (av[i][0] == 'p') {
            port = setkPortFormat(av[i] + 2);
        } else if (av[i][0] == 's') {
            bodySize = setMsgBodySize(av[i] + 2);
        } else if (av[i][0] == 'm') {
            method = setMethod(av[i] + 2);
        } else if (av[i][0] == 'u') {
            urn = av[i] + 2;
            std::cout << "urn       is set " << urn << std::endl;
        }
    }
    int sockfd = connectToServer(ip.c_str(), port.c_str());
    sendMSG(sockfd, bodySize, method, urn, ip + ":" + port);
    char buf[1];
    read(sockfd, buf, 1);
    close(sockfd);
    return 0;
}

long setMsgBodySize(char *size) {
    long b = checkSizeFormat(size);
    std::stringstream ss(size);

    long sizel;
    ss >> sizel;
    sizel *= b;
    std::cout << "body size is set " << sizel << std::endl;
    if (sizel > MAXBODY) {
        std::cerr << "Error: fix define MAXBODY or change size." << std::endl;
    }
    return sizel;
}

long    checkSizeFormat(char *size) {
    std::string sizestring;
    std::stringstream ss(size);
    ss >> sizestring;
    size_t pos = sizestring.find_first_not_of("0123456789MG");
    if (pos != std::string::npos) {
        std::cerr << "Error: wrong body size format (0123456789 and MG)." << std::endl;
        exit (1);
    }

    pos = sizestring.find_first_of("MG");
    if (pos != std::string::npos && pos != sizestring.length() - 1) {
        std::cerr << "Error: wrong body size format (MG can only be the last character)." << std::endl;
        exit (1);
    }

    if (pos == sizestring.length() - 1) {
        return(sizestring[pos] == 'M' ? 1000000 : 1000000000);
    }

    return 1;
}


std::string    setIpFormat(char *IPv4) {
    std::string IPv4str(IPv4);

    if (IPv4str.find_first_not_of("0123456789.") != std::string::npos) {
        std::cerr << "Error: wrong IP format (0123456789.)" << std::endl;
        exit (1);
    }

    int i = 4;
    size_t pos = 0;
    for (size_t poslast = 0; pos != std::string::npos && i; --i) {
        pos = IPv4str.find_first_of(".", poslast);

        size_t posn = (pos != std::string::npos ? pos: IPv4str.length());
        std::string ip(&IPv4str[poslast], &IPv4str[posn]);
        char *end;
        if (ip.empty() || strtoul(ip.c_str(), &end, 10) > 255) {
            std::cerr << "Error: wrong IP format (0-255)" << std::endl;
            exit (1);
        }
        poslast = pos + 1;
    }

    if (!(pos == std::string::npos && !i)) {
        std::cerr << (pos == std::string::npos) <<"Error: wrong IP format (x.x.x.x)" << i << std::endl;
        exit (1);
    }

    std::cout << "ip        is set " << IPv4 << std::endl;
    return IPv4str;
}

std::string    setkPortFormat(char *port) {
    std::string portstr(port);

    size_t pos = portstr.find_first_not_of("0123456789");
    if (pos != std::string::npos) {
        std::cerr << "Error: wrong port format (0123456789)." << std::endl;
        exit (1);
    }
    
    long portl;
    std::stringstream ss(port);
    ss >> portl;
    if (portl < 0 || portl > 65535) {
        std::cerr << "Error: wrong port value (0 - 65535)." << std::endl;
        exit (1);
    }

    if (portl < 1024 || portl > 49151) {
        std::cout << "Warning: port out of range 1024 - 49151." << std::endl;
    }
    
    std::cout << "port      is set " << port << std::endl;
    return port;
}

std::string    setMethod(char *methodChar) {
    std::string methodStr(methodChar);
    if (methodStr != "PUT" && methodStr != "POST") {
        std::cerr << "Error: wrong method (PUT or POST)." << std::endl;
        exit (1);
    }
    std::cout << "method is set " << methodStr << std::endl;
    return methodStr;
}

int connectToServer(const char *IPv4, const char *port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(IPv4, port, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(2);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
            std::cerr << "client: socket" << std::endl;
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::cerr << "client: connect. Try next addr" << std::endl;
            continue;
        }

        break;
    }

    if (p == NULL) {
        std::cerr << "client: failed to connect" << std::endl;
        exit(3);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    std::cout << "client: connecting to " <<  IPv4 << ":" << port << std::endl;
    freeaddrinfo(servinfo);

    return sockfd;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void    sendMSG(int sockfd, long stringsize, std::string method, 
                std::string urn, std::string hostport) {
    // sockfd = 1;
    // std::string body(stringsize, 'u');
    std::ostringstream ssss;
    ssss << stringsize;

    std::string headers;
    headers = method + " " + urn + " " + "HTTP/1.1\r\n"
                            "host: " + hostport + "\r\n"
                            // "Authorization: Basic bWh1ZmZsZXA6cXdlcXdlcXdl\r\n"
                            "Content-length: ";
    headers += ssss.str();
    headers += "\r\n\r\n";

    // print headers
    std::cout << std::endl << headers.c_str() << std::endl;

    // send headers
    write(sockfd, headers.c_str(), headers.length());

    // send body
    long i = 0;
    std::string body;
    long delim = 20;
    long sizeChunk = 0;
    long nbMsgPart = 0;
    if (stringsize/delim) {
        sizeChunk = stringsize/delim;
        body.assign(sizeChunk, 'u');
        for (; i + sizeChunk <= stringsize; i += sizeChunk) {
            write(sockfd, body.c_str(), sizeChunk);
            ++nbMsgPart;
            std::cout << "Client sended message part " << nbMsgPart << ", size " << sizeChunk * nbMsgPart << "/" << stringsize << std::endl;
        }
    }
    if (i < stringsize) {
        body.assign(stringsize - i, 'e');
        write(sockfd, body.c_str(), stringsize - i);
        std::cout << "Client sended message part " << nbMsgPart << ", size " << sizeChunk * nbMsgPart + stringsize - i << "/" << stringsize << std::endl;
    }
    std::cout << "Client sended message" << std::endl;
}
