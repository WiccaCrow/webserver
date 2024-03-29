import os
import fileinput
import sys

headers = []
body = """<html>
<head>
    <title>Server-provided Environment variables</title>
</head>
<body>
    <table>
        <th><td colspan=2 align=center>Environment Variables</td></th>
"""
for param in os.environ.keys():
    body += "<tr><td>%s</td><td>%s</td></tr>" % (param,os.environ[param])
body += "</table><br>"

body +="<label>Request Body: </label><p>"

bytesN = int(os.environ["CONTENT_LENGTH"])
if (bytesN != 0):
    body += sys.stdin.read(bytesN)

body += "</p></body></html>"

headers.append("Content-Type: text/html")
headers.append("Content-Length: %d" % len(body))

for header in headers:
    print(header)
print("")
print(body)
