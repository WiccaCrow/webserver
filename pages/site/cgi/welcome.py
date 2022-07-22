from gtts import gTTS
from io import BytesIO
import sys
import os

bytesN = int(os.environ["CONTENT_LENGTH"])
msg = 'Welcome on webserv'

if (bytesN != 0):
    msg = sys.stdin.read(bytesN)

mp3_bio = BytesIO()
tts = gTTS(text=msg, lang='en', slow=False)

tts.write_to_fp(mp3_bio)

size = len(mp3_bio.getvalue())
s = f"Accept-Ranges:none\nContent-type: audio/mpeg\nContent-length: {size}\n\n"

headers = BytesIO(s.encode())
sys.stdout.buffer.write(headers.getvalue())
sys.stdout.buffer.write(mp3_bio.getvalue())
