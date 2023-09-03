import os
import subprocess

headerCommand = r"gcc -Wall -fsyntax-only -g tempmain.c -I lib\GLFW\include -I lib\GLEW\include -I lib\cglm\include -I lib\CIMGUI -L lib\GLEW\lib\Release\x64 -L lib\GLFW\lib-mingw-w64 -lglew32s -l glfw3 -lgdi32 -lopengl32"
cFileCommand = r"gcc -Wall -fsyntax-only -g src\C_FILE -I lib\GLFW\include -I lib\GLEW\include -I lib\cglm\include -I lib\CIMGUI -L lib\GLEW\lib\Release\x64 -L lib\GLFW\lib-mingw-w64 -lglew32s -l glfw3 -lgdi32 -lopengl32"
relativePath = "src/" # the relative path to the source files

mainFunc = "int main(void) { return 0; }"
tempFileName = "tempmain.c"


if not os.path.isfile(tempFileName):
    tempFile = open(tempFileName, "x")
else:
    tempFile = open(tempFileName, "w")

directory = os.fsencode(relativePath)

for file in os.listdir(directory):
    filename = os.fsdecode(file)
    output = ''

    if filename.endswith(".h"):
        code = f'#include "src\\{filename}"\n{mainFunc}'
        tempFile.seek(0)
        tempFile.truncate(0)
        tempFile.write(code)
        output = subprocess.check_output(headerCommand, shell=True, stderr=subprocess.STDOUT)
    elif filename.endswith(".c"):
        output = subprocess.check_output(cFileCommand.replace("C_FILE", filename), shell=True,
            stderr=subprocess.STDOUT)

    if (output == b''):
        print(f"Compiled '{filename}'")
    else:
        print(f"Compiled '{filename}' with output!\n")
        print(output.decode('utf-8'))

tempFile.close()
os.remove(tempFileName)