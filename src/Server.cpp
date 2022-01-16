#include "Server.hpp"

// COUNT_SERVERS - количество серверов
#define COUNT_SERVERS 1

//typedef struct pollfd pollfd;

enum ServerError { 
	// WINSOCK_ERR = -1,
	// GETADDRINFO_ERR = -1,
	SOCKET_ERR  = -2, 
	SETOPT_ERR  = -3, 
	PARSE_ERR   = -4, 
	BIND_ERR    = -5, 
	LISTEN_ERR  = -6, 
	CONNECT_ERR = -7 
};

/******************************************************************************/
/* Constructors */

Server::Server() :
_addr("127.0.0.1"), _port(7676) {
	/**** заполнить переменные ****/
	assignPollFds();
	/**** сокет ****/
	createListenSock();
	/**** повторно использовать порт ****/
	reuseAddr();
	/**** связать порт и ip, чтобы listen происходил через этот порт ****/
	bindAddr();
	listenSock();

	_pollfds[0].fd = _servfd;
	_pollfds[0].events = POLLIN;
};


//      init
Server::Server(const std::string &ipaddr, const uint16_t port) :
_addr(ipaddr), _port(port) {
	/**** заполнить переменные ****/
	assignPollFds();
	/**** сокет ****/
	createListenSock();
	/**** повторно использовать порт ****/
	reuseAddr();
	/**** связать порт и ip, чтобы listen происходил через этот порт ****/
	bindAddr();
	listenSock();

	_pollfds[0].fd = _servfd;
	_pollfds[0].events = POLLIN;
}

//      copy
Server::Server(const Server &obj) {
    operator=(obj);
}


/******************************************************************************/
/* Destructors */
Server::~Server() {	
	const size_t size = _pollfds.size();
	for (size_t i = 0 ; i < size ; i++)
		close(_pollfds[i].fd);
}

/******************************************************************************/
/* Operators */

//      =
Server & Server::operator=(const Server &obj) {
    if (this != &obj) {
        _addr = obj._addr;   // Потом будет браться из конфига
        _port = obj._port; 
        _servfd = obj._servfd;
    	_pollResult = obj._pollResult;
        _pollfds = obj._pollfds;
    }
    return (*this);
}

/******************************************************************************/
/* Private functions */

void	Server::assignPollFds(void) {
	// _pollfds.reserve()
	_pollfds.assign(128, (struct pollfd) {
		-1,
		POLLIN | POLLOUT,
		0
	});
	_pollResult = 0;
}

void	Server::createListenSock(void) {
	_servfd = socket(PF_INET, SOCK_STREAM, 0);
	if (_servfd < 0) {
		std::cerr << "Cannot create listening socket" << std::endl; 
		exit(SOCKET_ERR);
	}
}

void	Server::reuseAddr(void) {
	int i = 1;
	if (setsockopt(_servfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
		std::cerr << "Cannot set options of the listening socket" << std::endl;
		exit(SETOPT_ERR);
	}
}

void	Server::bindAddr(void) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = inet_addr(_addr.c_str());
	if (bind(_servfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "Cannot bind listening socket" << std::endl; 
		exit(BIND_ERR);
	}
}

void	Server::listenSock(void) {
	if (listen(_servfd, SOMAXCONN) < 0) {
		std::cerr << "Listen failed" << std::endl; 
		exit(LISTEN_ERR);
	}
}

/******************************************************************************/
/* Public functions */

	/* Set atributs */

void    Server::resetPollEvents(void) {
    // <Wicca> : У кого не 0, сбрасываю на 0.
    if (_pollResult) {
        const size_t size = _pollfds.size();
        for (size_t i = 0 ; i < size ; i++)
            _pollfds[i].revents = 0;
        _pollResult = 0;
    }
}

	/* Get and show atributs */
int    Server::getServFd(void) {
	return (_servfd);
}

	/* other methods */
void    Server::start(void) {
    
    while (1) {
    	resetPollEvents();
        pollServ();
        const size_t size = _pollfds.size();
        for (size_t id = 0; id < size; id++) {
			if (id < COUNT_SERVERS && _pollfds[id].revents & POLLIN) {
            	acceptNewClient();
        	}
			else if (_pollfds[id].revents & POLLHUP) {
				std::cerr << "Cannot read from " << _pollfds[id].fd << " fd" << std::endl;
			}
			else if (_pollfds[id].revents & POLLOUT) {
                sendServ(id);
            }
            else if (_pollfds[id].revents & POLLIN) {					
                recvServ(id);
            }
			else if (_pollfds[id].revents & POLLERR) {
				std::cerr << "Poll internal error" << std::endl;
				// close all fds
				exit(1);
			}
        }
    }
}

void    Server::pollServ(void) {
    while (_pollResult == 0) {
        _pollResult = poll(_pollfds.data(), _pollfds.size(), 10000);
    }
    if (_pollResult < 0) {
		std::cerr << "Poll internal error" << std::endl;
		// close all fds
		exit(1);
		
        // выдать ошибку
    }
}

void    Server::recvServ(size_t i) {
	_pollfds[i].revents = 0;
	char	buf[PACKET_SIZE] = {0};
	
	if (_pollfds[i].fd == -1) {
		return ;
	}

	int	recvBytes = recv(_pollfds[i].fd, buf, PACKET_SIZE - 1, 0);
	if (recvBytes == 0) {
		disconnectClient(i);
	}
	if (recvBytes < 0) {
		disconnectClient(i);
		; //error case
	}
	if (recvBytes > 0) {
	// принять запрос и сформировать ответ
		

		std::string _remainder = "";
		// buf -> to_lower (only headers)
		// replace \r\n with \n
		_remainder += buf;
		// Get line		   <---| while _remainder != ""
		// Parse & Validate ---|

		// Parse first line
		std::string line = "GET / HTTP/1.1";
		Request req(line); // <---|
                           //     |
		// Parse headers ---------| inside this class 
		//header (struct maybe): key, value
		//split header line by ':' and trim whitespaces of each part
		//then insert into hash-table or list or tree
		
		// Parse body (if exist)

		//Request:  method, path, protocol, headers, [body], 
		//Reponse:  protocol, status(code), status(message)  headers, [body]

		// hashtable -> location ???

		// если от клиента пришел запрос, обработать 
		// флажок revents сменится на POLLOUT для выдачи ответа recv
		
	}
}

void    Server::sendServ(size_t id) {

	if (_pollfds[id].fd == -1) {
		return ;
	}

	// определить размер данных, которые надо отправить
	// sendByte (по аналогии с recvServ) или sendSize
	char	buf[PACKET_SIZE] = "message to send\n";
    int sendSize = strlen(buf);
    if (sendSize > PACKET_SIZE)
        sendSize = PACKET_SIZE;
	// если нет каких-то полей с указанием окончания отправки ответа,
	// клиент будет продолжать стоять в ожидании окончания ответа - POLLOUT
    int	send_byte = send(_pollfds[id].fd, buf, sendSize, 0);
    if (send_byte < 0) {
        disconnectClient(id);
        ; // error case
    }
    if (send_byte == 0) {
        disconnectClient(id);
    }
	if (send_byte > 0) {

		;// выдача ответа send
	}
}

static int isFreeFD(struct pollfd pfd) { 
	return pfd.fd == -1; 
}

void Server::acceptNewClient(void)
{
	struct sockaddr_in cliaddr;
	socklen_t addrlen = sizeof(cliaddr);

	int client_socket = accept(getServFd(), (struct sockaddr *)&cliaddr, &addrlen);
	if (client_socket > -1)
	{
		std::vector<struct pollfd>::iterator it = std::find_if(_pollfds.begin(), _pollfds.end(), isFreeFD);
		
		if (it != _pollfds.end()) {
			it->fd = client_socket;
			it->events = POLLIN | POLLOUT;
			it->revents = 0;
		}
		else {
			_pollfds.push_back((struct pollfd) {
				client_socket,
				POLLIN | POLLOUT,
				0
			});
		}
	}
}

void Server::disconnectClient(size_t id)
{
	if (_pollfds[id].fd != -1) {
		close(_pollfds[id].fd);
		_pollfds[id].fd = -1;
		_pollfds[id].events = 0;
		_pollfds[id].revents = 0;
	}
}