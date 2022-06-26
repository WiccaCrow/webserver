#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {

    // struct sockaddr_in addr;
    // addr.sin_family = AF_INET;
    // addr.sin_port = htons(1666);
    // inet_pton(AF_INET, "localhost", &addr.sin_addr);
    // socklen_t len = sizeof(addr);
    addrinfo *list = NULL;
    int res = 0;
    if ((res = getaddrinfo("localhost", "1666", NULL, &list))) {
        std::cerr << "getaddrinfo failed" << std::endl;
        exit(1);
    }

    std::cout << list->ai_socktype << " " << SOCK_STREAM << std::endl;
    int fd = socket(list->ai_family, SOCK_STREAM, 0);
    if (fd == -1) {
        std::cerr << "socket failed" << std::endl;
        exit(1);
    }

    res = connect(fd, list->ai_addr, list->ai_addrlen);
    if (res < 0) {  
        std::cerr << "connect failed" << std::endl;
        exit(1);
    }
    // std::string s = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    // write(fd, s.c_str(), s.length());

    // char buf[1000] = {0};
    // read(fd, buf, 1000);

    return 0;
}