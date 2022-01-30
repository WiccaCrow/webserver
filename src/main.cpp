#include "Server.hpp"

void checkArgs(int &ac, char **av) {
    if (ac != 2) {
        Log.error("Usage: " + std::string(av[0]) + " config.json");
        exit(1);
    }
}

// void *startInThread(void *ptr) {
//     Server *serv = reinterpret_cast<Server *>(ptr);
//
//     if (serv != NULL)
//         serv->start();
//     return NULL;
// }

int main(int ac, char **av) {
    checkArgs(ac, av);
    //Conf::load(av[1]);

    Log.setFlags(INFO | ERROR);
    Log.setLogFile("test.log");

    // Reading config file
    // Create threads (main + nbservBlock)

    // main - poll. read/write ops
    // other - permorm request and send res to main

    Server server;
    server.addServerBlocks("127.0.0.1", 8080);
    server.start();

    // if (serverBlocksCount > 1) {
    //        pthread_t thread[serverBlocksCount];
    //        for (size_t i = 0; i < servers.size(); i++) {
    //            pthread_create(&thread[i], NULL, startInThread, (void *)servers[i]);
    //        }
    //
    //        for (size_t i = 0; i < servers.size(); i++) {
    //            pthread_join(thread[i], NULL);
    //        }
    //    }
    //
    // Just a plain realization of the server
    //

    return 0;
}
