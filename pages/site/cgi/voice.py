from gtts import gTTS
from io import BytesIO
from io import TextIOWrapper
import codecs

text = 'Welcome on webserv'
filename = 'voice.mp3'

bio = BytesIO()
tts = gTTS(text=text, lang='en', slow=False)

tts.save(filename)

f = open(filename, "rb")

print("Content-type: audio/mpeg")
print("")

print(f.read().decode(''))

f.close()

# cp 850 OK
# iso-8859-1
# print(bio.getvalue())