#include "CmdArgs.hpp"

void
printUsage(void) {
    Log.info() << "Usage: webserv [conf.json] [-flags]" << Log.endl;
    Log.info() << "Config:" << Log.endl;
    Log.info() << HTAB << "path - absolute or relative path to json config" << Log.endl;
    Log.info() << "\t" << "default config path: " << DEFAULT_CONF_PATH << Log.endl;
    Log.info() << "Flags:" << Log.endl;
    Log.info() << " -h : help :)" << Log.endl;
    Log.info() << " -l : log level, values:" << Log.endl;
    Log.info() << HTAB << "syserr - triggers on syscalls failures" << Log.endl;
    Log.info() << HTAB << "error  - non-critical processing errors" << Log.endl;
    Log.info() << HTAB << "info   - some info about processed requests" << Log.endl;
    Log.info() << HTAB << "debug  - info useful for debugging" << Log.endl;
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
    if (!strcmp(val, "syserr")) {
        logLevel = LOG_SYSERR;
    } else if (!strcmp(val, "error")) {
        logLevel = LOG_ERROR;
    } else if (!strcmp(val, "info")) {
        logLevel = LOG_INFO;
    } else if (!strcmp(val, "debug")) {
        logLevel = LOG_DEBUG;
    } else {
        printUsage();
        return 0;
    }

    Log.setLevel(logLevel);    
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
parseArgs(char **av) {
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
        } else if (!strcmp(av[i], "--daemon")) {
            isDaemon = true;
            i++;
        } else if (i == 1 && !endsWith(av[i], ".json")) {
            Log.error() << "Unknown parameter " << av[i] << Log.endl;
            printUsage();
            return 0;
        }
    }
    return 1;
}
