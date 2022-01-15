#include "Server.hpp"

void	checkArgs(int &ac, char **av) {
	if (ac > 2)
	{
		std::cerr << "Usage: " << av[0] << "config.json" << std::endl;
		exit(1);
	}
}

void	*startInThread(void *ptr) {
	Server *serv = reinterpret_cast<Server *>(ptr);
	
	if (serv != NULL)
		serv->start();
	 return NULL;
}

int	main(int ac, char **av)
{
	checkArgs(ac, av);

	// Reading config file
	const int serverBlocksCount = 1;

    std::vector<Server *> servers(serverBlocksCount);
    // Fill array with servers
    if (serverBlocksCount > 1) {
        pthread_t thread[serverBlocksCount];
        for (size_t i = 0; i < servers.size(); i++)
        {
            pthread_create(&thread[i], NULL, startInThread, (void *)servers[i]);
        }

        for (size_t i = 0; i < servers.size(); i++)
        {
            pthread_join(thread[i], NULL);
        }
    }
    else {
        // Just a plain realization of the server
		servers[0] = new Server;
        servers[0]->start();
    }

    return 0;
}
