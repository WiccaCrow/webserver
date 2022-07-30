import os
import sys
from gtts import gTTS
from io import BytesIO

bytesN = int(os.environ["CONTENT_LENGTH"])
msg = 'Welcome on webserv '

if (bytesN != 0):
    body = sys.stdin.read(bytesN)
    body = body.replace('fname=', '')
    msg += body + "!"

tts = gTTS(text=msg, lang='en', slow=False)

tts.save('./voice.mp3')

print("Content-length: 0")
print("Location: /welcome/welcome.html\n")

