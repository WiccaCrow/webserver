#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int ac, char **av, char **env) {

    printf("Content-type: text/html\r\n\r\n");
    size_t count = 0;
    count += printf("<html>\n");
    count += printf("<title>Server-provided Environment variables</title>");
    count += printf("<body>\n");
    count += printf("<table>\n");
    count += printf("<th><td colspan=2 align=center>Environment Variables</td></th>\n");
    char *cl = NULL;
    for (size_t i = 0; env[i] != NULL; i++)
    {
        // fprintf(stderr, "script: %s\n", env[i]);
        char *token = strtok(env[i], "=");
        char *value = strtok(NULL, "=");
        if (!strcmp(token, "CONTENT_LENGTH")) {
            cl = value;
        }
        count += printf("<tr><td>%s</td><td>%s</td></tr>\n", token, value);
    }
    count += printf("</table></body></html>\n");

    size_t size = atoi(cl);
    if (cl == NULL) {
        fprintf(stderr, "script:no CONTENT_LENGTH");
        return 0;
    } else {
        size = atoi(cl);
        fprintf(stderr, "script: len: %zu\n", size);
    }

    char buf[1024]; // Could be not enough
    int bytes = read(0, buf, size);
    if (bytes < 0) {
        // fprintf(stderr, "script:read failed\n");
        return -1;
    } else if (bytes == 0) {
        // fprintf(stderr, "script:read eof\n");
    } else {
        buf[bytes] = 0;
        // fprintf(stderr, "script:readed %s\n", buf);
    }
    count += printf("Body: %s\n", buf);
    // fprintf(stderr, "script:body size %zu\n", count);
    
    // int bytes = read(0, buf, size);
    return 0;
}