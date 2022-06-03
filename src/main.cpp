#include "Config.hpp"
#include "Globals.hpp"

Server *g_server;

int
main(int ac, char **av) {

    Log.setFlags(LOG_INFO | LOG_ERROR | LOG_DEBUG);
    // Log.setLogFile("test.log");

    if (ac > 2) {
        Log.error("Usage: " + std::string(av[0]) + " [config.json]");
        return 1;
    }

    g_server = loadConfig(ac == 1 ? "./conf/default.json" : av[1]);
    if (!g_server) {
        return 1;
    }
    Log.info("Config is loaded");

    g_server->start();

    delete g_server;
    return 0;
}
