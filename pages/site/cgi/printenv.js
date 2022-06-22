let headers = [];
let body;

body = "<html>\n";
body += "<title>Server-provided Environment variables</title>";
body += "<body>\n";
body += "<table>\n";
body += "<th><td colspan=2 align=center>Environment Variables</td></th>\n";
for (let key in process.env) {
    body += `<tr><td>${key}</td><td>${process.env[key]}</td></tr>\n`;
}
body += "</table></body></html>\n";

headers.push("Content-type: text/html");
headers.push(`Content-length: ${body.length}`);

headers.forEach(header => {
    console.log(header);
});
console.log();
console.log(body);