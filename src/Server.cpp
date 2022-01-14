#include "Server.hpp"

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

//      init
Server::Server(const std::string &ipaddr = "127.0.0.1", const uint16_t port = 7676) :
	_addr(ipaddr),                       
	_port(port)
{
/**** заполнить переменные ****/
	_pollfds.reserve(128);

/**** сокет ****/
	_servfd = socket(PF_INET, SOCK_STREAM, 0);
	if (_servfd < 0)
	{
		std::cerr << "Cannot create listening socket" << std::endl; 
		exit(SOCKET_ERR);
	}

/**** повторно использовать порт ****/
	int i = 1;
	if (setsockopt(_servfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
		std::cerr << "Cannot set options of the listening socket" << std::endl;
		exit(SETOPT_ERR);
	}

/**** связать порт и ip, чтобы listen происходил через этот порт ****/
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = inet_addr(_addr.c_str());
	if (bind(_servfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "Cannot bind listening socket" << std::endl; 
		exit(BIND_ERR);
	}

	if (listen(_servfd, SOMAXCONN) < 0) {
		std::cerr << "Listen failed" << std::endl; 
		exit(LISTEN_ERR);
	}

/**** подготовить данные для дальнейших опросов poll ****/

	_pollfds.push_back((struct pollfd) {
		_servfd,
		POLLIN,
		0
	});
}

//      copy

/******************************************************************************/
/* Destructors */
Server::~Server()
{	
	const size_t size = _pollfds.size();
	for (size_t i = 0 ; i < size ; i++)
		close(_pollfds[i].fd);
}

/******************************************************************************/
/* Operators */

//      =

/******************************************************************************/
/* Private functions */


/******************************************************************************/
/* Public functions */

	/* Set atributs */

void    Server::resetPollEvents(void)
{
	const size_t size = _pollfds.size();
	for (size_t i = 0 ; i < size ; i++)
		_pollfds[i].revents = 0;
}

	/* Get and show atributs */
int    Server::getServFd(void)
{
	return (_servfd);
}

	/* other methods */
void    Server::startServ(void)
{
	int pollResult = 0;
	while (1)
	{
		if (pollResult) {
            // <Wicca> : в дальнейшем контейнер серверов, 
            // массив pollResult [total_nb_serv]. У кого не 0, сбрасываю на 0.
			resetPollEvents();
			pollResult = 0; 
		}

		while (pollResult == 0) {
			pollResult = poll(&_pollfds.front(), _pollfds.size(), 0);
		}

		if (pollResult > 0) {
			
			// пройтись по всем fd // отслеживаю изменения у клиентов
			const size_t size = _pollfds.size();
	        for (size_t i = 1 ; i < size ; i++) {
				if (_pollfds[i].revents & POLLIN) {					
					recvServ(_pollfds[i], (int)i);
				} 
				else if (_pollfds[i].revents & POLLOUT) {
					// выдача ответа send
					sendServ(_pollfds[i], (int)i);
				}
			}

			// если что-то изменилось на общем fd (cлушающем сокете)
			if (_pollfds[0].revents & POLLIN) {
				acceptNewClient();
			}
		}
		else {
			// выдать ошибку
		} 
	}
}

void    Server::recvServ(struct pollfd pollCli, int i)
{
	pollCli.revents = 0;
	char	buf[TCP_SIZE];
	memset(&buf[TCP_SIZE], 0, TCP_SIZE);
	int	recvByte = recv(pollCli.fd, buf, TCP_SIZE - 1, 0);
	if (recvByte == 0) {
		disconnectClient(i);
	}
	if (recvByte < 0) {
		disconnectClient(i);
		; //error case
	}
	if (recvByte > 0) {
	// принять запрос и сформировать ответ

		// Read data by line (recv)
		// Replace \r\n with \n (???)
		// Validate line

		/*
		// Parse first line
		// getting method
		// getting path 
		// getting protocol
		
		// Parse headers
		header (struct maybe): key, value
		split header line by ':' and trim whitespaces of each part
		then insert into hash-table or list or tree
		
		// Parse body (if exist)

		Request:  method, path, protocol, headers, [body], 
		Reponse:  protocol, status(code), status(message)  headers, [body]

		// hashtable -> location

		// если от клиента пришел запрос, обработать 
		// флажок revents сменится на POLLOUT для выдачи ответа recv
		*/
	}
}

void    Server::sendServ(struct pollfd pollCli, int i)
{
	// определить размер данных, которые надо отправить 
	// sendByte (по аналогии с recvServ) или sendSize
    int sendSize;
    if (sendSize > TCP_SIZE)
        sendSize = TCP_SIZE;
    int	send_byte = ::send(pollCli.fd, "сообщение для отправки", sendSize, 0);
    if (send_byte < 0){
        disconnectClient(i);
    }
    if (send_byte == 0) {
        disconnectClient(i);
        ; // error case
    }
	if (send_byte > 0) {
        // disconnectClient(i)
		;// выдача ответа send
	}
}

void Server::acceptNewClient(void)
{
	struct sockaddr_in cliaddr;
	socklen_t addrlen = sizeof(cliaddr);

	int client_socket = accept(getServFd(), (struct sockaddr *)&cliaddr, &addrlen);
	if (client_socket > -1)
	{
		_pollfds.push_back((struct pollfd) {
			client_socket,
			POLLIN | POLLOUT,
			0
		});
	}
}

void Server::disconnectClient(int i)
{
	close(_pollfds[i].fd);
	_pollfds.erase(_pollfds.begin() + i);
}