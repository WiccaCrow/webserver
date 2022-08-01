// g++ -Wall -Wextra -Werror puttest.cpp -o puttest ; ./puttest 127.0.0.1 123G
// ./puttest 127.0.0.1 123
// ./puttest 127.0.0.1 123M
// ./puttest 127.0.0.1 123G

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


#define PORT "8080"
#define MAXBODY 20000000000

long    setMsgBodySize(char *size);
void*   get_in_addr(struct sockaddr *sa);
int     connectToServer(char *IPv4);
void    sendMSG(int sockfd, long stringsize);
void    checkIpFormat(char *IPv4);
long    checkSizeFormat(char *size);

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

long setMsgBodySize(char *size) {
    long b = checkSizeFormat(size);
    std::stringstream ss(size);

    long sizelong;
    ss >> sizelong;
    sizelong *= b;
    std::cout << "body size is set " << sizelong << std::endl;
    if (sizelong > MAXBODY) {
        std::cerr << "Error: fix define MAXBODY or change size." << std::endl;
    }
    return sizelong;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void    checkIpFormat(char *IPv4) {
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
}

int connectToServer(char *IPv4) {
    checkIpFormat(IPv4);

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
