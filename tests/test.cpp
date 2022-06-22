// #include <string>
// #include <iostream>

// #include "URI.hpp"

// int main(int ac, char **av) {
//     if (ac < 2) {
//         std::cout << "Err: nb argv" << std::endl;
//         return 1;
//     }

//     std::cout << av[1] << std::endl;
//     const std::string &s = HTTP::URI::encode(av[1]);
//     std::cout << s << std::endl;
//     std::cout << HTTP::URI::decode(s) << std::endl;
//     return 0;
// }

#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

int main(void)
{
    struct addrinfo *result;
    struct addrinfo *p;
    int error;

    // char str[1024] = {0};
    // gethostname(str, 1024);
    std::string str("google.com");
    error = getaddrinfo(str.c_str(), "80", NULL, &result);
    if (error != 0)
    {   
        fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(error));
        return EXIT_FAILURE;
    }   

    for (p = result; p != NULL; p = p->ai_next)
    {   
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *) p->ai_addr;
            char ip[INET_ADDRSTRLEN];

            printf("family: %8s | socktype: %11s | "
                   "proto: %2d | addr: %25s | port: %d\n",
                   "AF_INET", 
                   p->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" : p->ai_socktype == SOCK_DGRAM ? "SOCK_DGRAM" : "UNDEF",
                   p->ai_protocol, 
                   inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN),
                   ntohs(addr->sin_port)
                   );
        } else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *) p->ai_addr;
            char ip[INET6_ADDRSTRLEN];

            printf("family: %8s | socktype: %11s | "
                   "proto: %2d | addr: %25s | port: %d\n",
                   "AF_INET6",  
                   p->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" : p->ai_socktype == SOCK_DGRAM ? "SOCK_DGRAM" : "UNDEF" ,
                   p->ai_protocol,
                   inet_ntop(AF_INET6, &addr->sin6_addr, ip, INET6_ADDRSTRLEN),
                   ntohs(addr->sin6_port)
                   );
        }
    }   


    freeaddrinfo(result);
    return EXIT_SUCCESS;
}


/*
** client.c - a stream socket client demo
*/

// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <errno.h>
// #include <string.h>
// #include <netdb.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <sys/socket.h>

// #include <arpa/inet.h>

// #define PORT "3490" // Порт, к которому подключается клиент

// #define MAXDATASIZE 100 // максимальное число байт, принимаемых за один раз

// // получение структуры sockaddr, IPv4 или IPv6:
// void *get_in_addr(struct sockaddr *sa) {
//     if (sa->sa_family == AF_INET) {
//         return &(((struct sockaddr_in*)sa)->sin_addr);
//     }

//     return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }

// int main(int argc, char *argv[]) {
//     int sockfd, numbytes;
//     char buf[MAXDATASIZE];
//     struct addrinfo hints, *servinfo, *p;
//     int rv;
//     char s[INET6_ADDRSTRLEN];

//     if (argc != 2) {
//         fprintf(stderr,"usage: client hostnamen");
//         exit(1);
//     }

//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;

//     if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
//         fprintf(stderr, "getaddrinfo: %sn", gai_strerror(rv));
//         return 1;
//     }

//     // Проходим через все результаты и соединяемся к первому возможному
//     for(p = servinfo; p != NULL; p = p->ai_next) {
//         if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
//             perror("client: socket");
//             continue;
//         }

//         if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
//             close(sockfd);
//             perror("client: connect");
//             continue;
//         }

//         break;
//     }

//     if (p == NULL) {
//         fprintf(stderr, "client: failed to connectn");
//         return 2;
//     }

//     inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
//     s, sizeof s);
//     printf("client: connecting to %sn", s);

//     freeaddrinfo(servinfo); // эта структура больше не нужна

//     if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
//         perror("recv");
//         exit(1);
//     }

//     buf[numbytes] = "'"[0];

//     printf("client: received '%s'n",buf);

//     close(sockfd);

//     return 0;
// }
