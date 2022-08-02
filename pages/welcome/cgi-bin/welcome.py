import os
import sys
from gtts import gTTS
from io import BytesIO
import urllib
import cgi,cgitb

form = cgi.FieldStorage()

msg = 'Welcome on webserv ' + form['fname'].value

# print(msg, file=sys.stderr)

tts = gTTS(text = msg, lang = 'en', slow = False)

tts.save('../welcome.mp3')

print("Content-length: 0")
print("Location: /welcome/welcome.html\n")

