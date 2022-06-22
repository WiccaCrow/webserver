import os
import fileinput

headers = []
body = """
<html>
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
for line in fileinput.input():
    body += line
body += "</p></body></html>"

headers.append("Content-type: text/html")
headers.append("Content-Length: %d" % len(body))

for header in headers:
    print(header)
print("")
print(body)
