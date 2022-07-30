let headers = [];
let body;

body = "<html>";
body += "<title>Server-provided Environment variables</title>";
body += "<body>";
body += "<table>";
body += "<th><td colspan=2 align=center>Environment Variables</td></th>";
for (let key in process.env) {
    body += `<tr><td>${key}</td><td>${process.env[key]}</td></tr>`;
}
body += "</table></body></html>";

headers.push("Content-Type: text/html");
headers.push(`Content-Length: ${body.length}`);

headers.forEach(header => {
    console.log(header);
});
console.log();
console.log(body);