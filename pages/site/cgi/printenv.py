import os

print "Content-type: text/html\n\n"
print "<html>\n"
print "<title>Server-provided Environment variables</title>"
print "<body>\n"
print "<table>\n"
print "<th><td colspan=2 align=center>Environment Variables</td></th>\n"
for param in os.environ.keys():
    print "<tr><td>%s</td><td>%s</td></tr>\n" % (param,os.environ[param])
print "</table></body></html>\n"