#include "Config.hpp"
#include "Globals.hpp"
#include "Utils.hpp"

Server *g_server;

void
printUsage(void) {
    Log.info() << "Usage: webserv [conf.json] [-flags]" << Log.endl;
    Log.info() << "Config:" << Log.endl;
    Log.info() << HTAB << "path - absolute or relative path to json config" << Log.endl;
    Log.info() << HTAB << "default config path: " << DEFAULT_CONF_PATH << Log.endl;
    Log.info() << "Config:" << Log.endl;
    Log.info() << "Flags:" << Log.endl;
    Log.info() << " -h : help :)" << Log.endl;
    Log.info() << " -l : log level, values:" << Log.endl;
    Log.info() << HTAB << "d - debug" << Log.endl;
    Log.info() << HTAB << "i - info" << Log.endl;
    Log.info() << HTAB << "e - error" << Log.endl;
    Log.info() << HTAB << "s - system error" << Log.endl;
    Log.info() << " -d : logs directory, value:" << Log.endl;
    Log.info() << HTAB << "path - absolute or relative path to existing directory" << Log.endl;
}

int
parseLogLevel(const char *val) {

    if (val == NULL) {
        printUsage();
        return 0;
    }

    uint8_t logLevel = 0;
    for (std::size_t i = 0; val[i]; i++) {
        switch (val[i]) {
            case 'd': logLevel |= LOG_DEBUG; break;
            case 'i': logLevel |= LOG_INFO; break;
            case 'e': logLevel |= LOG_ERROR; break;
            case 's': logLevel |= LOG_SYSERR; break;
            default: {
                printUsage();
                return 0;
            }
        }
    }
    Log.setFlags(logLevel);    
    return 1;
}

int
parseLogDirectory(const char *val) {
    if (val == NULL) {
        printUsage();
        return 0;
    }
    if (!isDirectory(val)) {
        Log.error() << "Directory " << val << " does not exist " << Log.endl;
        return 0;
    }
    Log.setLogDir(val);
    return 1;
}

int
parseFlags(char **av) {
    for (std::size_t i = 1; av[i]; i++) {
        if (!strcmp(av[i], "-l")) {
            if (!parseLogLevel(av[i + 1])) {
                return 0;
            }
            i++;
        } else if (!strcmp(av[i], "-d")) {
            if (!parseLogDirectory(av[i + 1])) {
                return 0;
            }
            i++;
        } else if (!strcmp(av[i], "-h")) {
            printUsage();
            return 0;
        } else if (i == 1 && !endsWith(av[i], ".json")) {
            Log.error() << "Unknown parameter " << av[i] << Log.endl;
            printUsage();
            return 0;
        }
    }
    return 1;
}

int
main(int ac, char **av) {
    (void)ac;

    Log.setFlags(LOG_INFO | LOG_ERROR | LOG_DEBUG | LOG_SYSERR);
    if (!parseFlags(av)) {
        return 1;
    }
    Log.enableLogFile();

    std::string config = DEFAULT_CONF_PATH;
    if (av[1] && endsWith(av[1], ".json")) {
        config = av[1];
    }

    g_server = loadConfig(config);
    if (!g_server) {
        return 1;
    }
    Log.info() << "Config " << config << " is loaded" << Log.endl;

    g_server->start();

    delete g_server;
    return 0;
}
