#include "Config.hpp"

int main(int ac, char **av, char **env) {

    if (ac != 2) {
        Log.error("Usage: " + std::string(av[0]) + " conf.json");
        return 1;
    }

    Log.setFlags(INFO | ERROR | DEBUG);
    // Log.setLogFile("test.log");

    Server *server = loadConfig(av[1]);
    if (!server) {
        return 1;
    }
    Log.info("Config is loaded");

    server->env = env;
    server->start();

    delete server;
    return 0;
}
