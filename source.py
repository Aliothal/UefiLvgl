import os

lines = []

for root, dirs, files in os.walk(os.path.join(os.path.dirname(__file__),"src")):
    for file in files:
        if file.endswith('.c'):
            # print(file)
            lines.append(os.path.join(root, file) + '\n')

with open(os.path.join(os.path.dirname(__file__),"source.txt"), "w+") as f:
    f.writelines(lines)