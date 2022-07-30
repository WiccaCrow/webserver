import cgi,cgitb
import sys

print("script::1", file=sys.stderr)

cgitb.enable()

print("script::2", file=sys.stderr)

form = cgi.FieldStorage()

print("script::3", file=sys.stderr)

name = form.getvalue('sample')

print("Name of the user is:", name)