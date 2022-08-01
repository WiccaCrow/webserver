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


#define PORT "8080"

long    setMsgBodySize(char *size);
void*   get_in_addr(struct sockaddr *sa);
int     connectToServer(char *IPv4);
void    sendMSG(int sockfd, long stringsize);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error. Usage: client hostname (IPv4) and size of body to send (bytes)\n");
        exit(1);
    }
    
    long stringsize = setMsgBodySize(argv[2]);
    int sockfd = connectToServer(argv[1]);
    sendMSG(sockfd, stringsize);
    char buf[10];
    read(sockfd, buf, 10);
    close(sockfd);
    return 0;
}

long setMsgBodySize(char *size) {
    long stringsize;
    std::stringstream ss(size);
    ss >> stringsize;
    std::cout << "body size is set." << std::endl;
    return stringsize;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int connectToServer(char *IPv4) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(IPv4, PORT, &hints, &servinfo)) != 0) {
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
    std::cout << "client: connecting to " <<  IPv4 << ":" << PORT << std::endl;
    freeaddrinfo(servinfo);

    return sockfd;
}

void    sendMSG(int sockfd, long stringsize) {
    // sockfd = 1;
    // std::string body(stringsize, 'u');
    std::ostringstream ssss;
    ssss << stringsize;

    std::string headers;
    headers = "PUT /put/testdel2 HTTP/1.1\r\n"
                            "host: localhost:8080\r\n"
                            // "Authorization: Basic bWh1ZmZsZXA6cXdlcXdlcXdl\r\n"
                            "Content-length: ";
    headers += ssss.str();
    headers += "\r\n\r\n";

    // print headers
    write(1, headers.c_str(), headers.length());

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
