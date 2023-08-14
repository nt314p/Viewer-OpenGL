import os
import subprocess

headerCommand = r"gcc -Wall -fsyntax-only -g tempmain.c -I lib\GLFW\include -I lib\GLEW\include -I lib\cglm\include -I lib\CIMGUI -L lib\GLEW\lib\Release\x64 -L lib\GLFW\lib-mingw-w64 -lglew32s -l glfw3 -lgdi32 -lopengl32"
cFileCommand = r"gcc -Wall -fsyntax-only -g src\C_FILE -I lib\GLFW\include -I lib\GLEW\include -I lib\cglm\include -I lib\CIMGUI -L lib\GLEW\lib\Release\x64 -L lib\GLFW\lib-mingw-w64 -lglew32s -l glfw3 -lgdi32 -lopengl32"


mainFunc = "int main(void) { return 0; }"
tempFileName = "tempmain.c"


if not os.path.isfile(tempFileName):
    tempFile = open(tempFileName, "x")
else:
    tempFile = open(tempFileName, "w")

directory = os.fsencode("src/")

for file in os.listdir(directory):
    filename = os.fsdecode(file)
    if filename.endswith(".h"):
        print(filename)
        code = f'#include "src\\{filename}"\n{mainFunc}'
        tempFile.seek(0)
        tempFile.truncate(0)
        tempFile.write(code)
        output = subprocess.check_output(headerCommand, shell=True)
        print(output)
    elif filename.endswith(".c"):
        print(filename)
        output = subprocess.check_output(cFileCommand.replace("C_FILE", filename), shell=True)
        print(output)

tempFile.close()
os.remove(tempFileName)
