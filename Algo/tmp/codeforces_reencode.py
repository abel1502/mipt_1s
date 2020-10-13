import sys

data = sys.stdin.read()

data = data.replace("Скопировать\n", "\n")

data = data.encode("windows-1251", errors="namereplace")

output = open("encoded.txt", "wb")
output.write(data)
output.close()