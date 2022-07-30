#!/bin/perl
print "Content-type: text/html\n\n";
print "<html>\n";
print "<title>Server-provided Environment variables</title>";
print "<body>\n";
print "<table>\n";
print "<th><td colspan=2 align=center>Environment Variables</td></th>\n";
foreach my $x (keys %ENV) {
    print "<tr><td>$x</td><td>$ENV{$x}</td></tr>\n";
}
print "</table></body></html>\n";