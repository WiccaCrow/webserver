#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Request.hpp"

int main(int argc, char *argv[])
{
	FILE *stream;
	char *line = NULL;
	size_t len = 0;
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

	Request req;
	int i = 0;
	while ((nread = getline(&line, &len, stream)) != -1) {
		std::cout << line;
		if (!(req.getFlags() & PARSED_SL)) {
			req.parseStartLine(line);
		}
	}

	free(line);
	fclose(stream);
	exit(EXIT_SUCCESS);
}
