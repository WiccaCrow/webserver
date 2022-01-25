#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "Request.hpp"

int main(int argc, char *argv[]) {
    FILE *  stream;
    char *  line = NULL;
    size_t  len = 0;
    ssize_t nread;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    stream = fopen(argv[1], "r");
    if (stream == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    HTTP::Request    req;
    HTTP::StatusCode status;
    int              i = 0;
    while ((nread = getline(&line, &len, stream)) != -1) {
        // std::cout << line;
        if (!(req.getFlags() & PARSED_SL)) {
            if ((status = req.parseStartLine(line)) != HTTP::CONTINUE) {
                Log.error("Error"); // for test
                // return HTTP::Response(status);
            }
        } else if (!(req.getFlags() & PARSED_HEADERS)) {
            if ((status = req.parseHeader(line)) != HTTP::CONTINUE) {
                Log.error("Error"); // for test
                                    // return HTTP::Response(status);
            }
        } else if (!(req.getFlags() & PARSED_BODY)) {
            std::cout << "parse body");
            // if ((status = req.parseBody(line)) != HTTP::CONTINUE) {
            //     Log.error("Error"); // for test
            // 	// return HTTP::Response(status);
            // }
        }
    }

    free(line);
    fclose(stream);
    exit(EXIT_SUCCESS);
}
