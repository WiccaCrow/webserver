#include <poll.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>

int main() {


	std::vector<struct pollfd> fds;
	int	servfd = socket(PF_INET, SOCK_STREAM, 0);
	if (servfd < 0)
	{
		std::cerr << "Cannot create listening socket" << std::endl; 
		exit(1);
	}

	int i = 1;
	if (setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
		std::cerr << "Cannot set options of the listening socket" << std::endl;
		exit(1);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(7676);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (bind(servfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "Cannot bind listening socket" << std::endl; 
		exit(1);
	}

	if (listen(servfd, 128) < 0) {
		std::cerr << "Listen failed" << std::endl; 
		exit(1);
	}


	fds.push_back((struct pollfd) {
		servfd,
		POLLIN,
		0
	});

	int pollResult = 0;
    while (1)
    {
		if (pollResult) {
	        const size_t size = fds.size();
	        for (size_t i = 0 ; i < size ; i++)
	            fds[i].revents = 0;
	        pollResult = 0;
   		}
	    
		while (pollResult == 0) {
        	pollResult = poll(fds.data(), fds.size(), 0);
    	}
    	
		if (pollResult < 0) {
			std::cerr << "poll error" << std::endl;
        	// выдать ошибку
    	}

		const size_t size = fds.size();

		static int count = 0;
	    for (size_t id = 1 ; id < size; id++) {
	        if (fds[id].revents & POLLIN) {	
				char buf[2048] = {0};
				int b;
				if ( (b = recv(fds[id].fd, buf, 2048, 0)) < 0) {
					close(fds[id].fd);
					fds.pop_back();
					std::cerr << "recv failed" << std::endl;
				}
				else if ( b == 0 ) {
					std::cerr << "disconnected" << std::endl;
					close(fds[id].fd);
					fds.pop_back();
				}
				else
					std::cout << "buf: " << buf << std::endl;
				//char tmp[] = "HTTP/1.1 200 OK\r\nContent-type: text/html; charset=UTF-8\r\n\r\n<html><body><h1>Hello!</h1></body></html>\r\n\r\n\0";
				
				
				std::string body = "<html><body><h1>Hello!</h1></body></html>\r\n";

				std::string res = "HTTP/1.1 200 OK\r\n";
				//res += "Access-Control-Allow-Origin: *\r\n"; 
				res += "Connection: close\r\n"; 
				//res += "Content-Encondig: gzip\r\n"; 
				res += "Content-Type: text/html; charset=utf-8\r\n"; 
				//res += "Keep-Alive: timeout=5, max=999\r\n";
				//res += "Date: Sat, 15 Jan 2022 16:57:59 GMT\r\n";
				res += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
				res += body;
				
				int s;
				if ((s = send(fds[id].fd, res.c_str(), res.size(), 0)) < 0)
					std::cerr << "send failed" << std::endl;
				else if (s == 0) {

				}
				else {
	        		fds[id].revents = 0;
					std::cout << "sent " << count++ << std::endl;
					//shutdown(fds[1].fd, SHUT_RDWR);
				}
	        }
	        //else if (fds[id].revents & POLLOUT) {
			//}
			else if (fds[id].revents & POLLHUP) {
				std::cerr << "client hang up" << std::endl;
			}
			else if (fds[id].revents & POLLERR) {
				std::cerr << "poll err" << std::endl;
			}
	    }
	    if (fds[0].revents & POLLIN) {
			int client_socket = accept(servfd, 0, 0);
			fds.push_back((struct pollfd) {
				client_socket,
				POLLIN | POLLOUT,
				0
			});
	    }
	}

	return 0;
}


//buf: Hey

//buf: GET / HTTP/1.1
//Host: 127.0.0.1:7676
//Connection: keep-alive
//sec-ch-ua: " Not;A Brand";v="99", "Google Chrome";v="97", "Chromium";v="97"
//sec-ch-ua-mobile: ?0
//sec-ch-ua-platform: "Linux"
//Upgrade-Insecure-Requests: 1
//User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.71 Safari/537.36
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
//Sec-Fetch-Site: none
//Sec-Fetch-Mode: navigate
//Sec-Fetch-User: ?1
//Sec-Fetch-Dest: document
//Accept-Encoding: gzip, deflate, br
//Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7

