let headers, body;

body = "<html>\n";
body += "<title>Server-provided Environment variables</title>";
body += "<body>\n";
body += "<table>\n";
body += "<th><td colspan=2 align=center>Environment Variables</td></th>\n";
for (let key in process.env) {
    body += `<tr><td>${key}</td><td>${process.env[key]}</td></tr>\n`;
}
body += "</table></body></html>\n";

headers = "Content-type: text/html\n";
headers += `Content-length: ${body.length}\n\n`;

console.log(headers);
console.log(body);