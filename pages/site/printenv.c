#include <string.h>
#include <stdio.h>

int main(int ac, char **av, char **env) {

    printf("Content-type: text/html\r\n\r\n");
    printf("<html>\n");
    printf("<title>Server-provided Environment variables</title>");
    printf("<body>\n");
    printf("<table>\n");
    printf("<th><td colspan=2 align=center>Environment Variables</td></th>\n");
    for (size_t i = 0; env[i] != NULL; i++)
    {
        char *token = strtok(env[i], "=");
        char *value = strtok(NULL, "=");
        printf("<tr><td>%s</td><td>%s</td></tr>\n", token, value);
    }
    printf("</table></body></html>\n");

    return 0;
}