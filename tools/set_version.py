import sys
import os
import shutil


if len(sys.argv) < 2:
    print "not enough arguments"
    exit(1)

outFile = open("../src/version.inl", "w")
outFile.write("\"" + sys.argv[1] + "\"\n")
outFile.close()


shutil.move("build/vs/auxprojects/Installer/version.wxi", "build/vs/auxprojects/Installer/version.wxi.old")
outFile = open("build/vs/auxprojects/Installer/version.wxi", "w")
with open("build/vs/auxprojects/Installer/version.wxi.old", "r") as inFile:
    for line in inFile:
        if line.startswith("    <?define version="): 
            line = "    <?define version=\"" + sys.argv[1] + "\" ?>\n"
        outFile.write(line)
inFile.close()
outFile.close()
os.remove("build/vs/auxprojects/Installer/version.wxi.old")