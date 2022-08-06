#include "Config.hpp"
#include "CmdArgs.hpp"

Server *g_server;

bool isDaemon;

int main(int ac, char **av) {
    (void)ac;
    
    isDaemon = false;
    Log.setLevel(LOG_INFO);
    if (!parseArgs(av)) {
        return 1;
    }
    Log.logToFile(true);

    std::string config = DEFAULT_CONF_PATH;
    if (av[1] && endsWith(av[1], ".json")) {
        config = av[1];
    }

    g_server = loadConfig(config);
    if (!g_server) {
        return 1;
    }
    Log.info() << "Config " << config << " is loaded" << Log.endl;
    g_server->isDaemon = isDaemon;
    g_server->start();

    delete g_server;
    return 0;
}
