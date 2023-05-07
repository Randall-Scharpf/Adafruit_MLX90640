f = open("raw.log")
lines = f.readlines()
for line in lines:
    s = line.split("0C2060")
    if (len(s) == 2):
        print(s[1][:-1])