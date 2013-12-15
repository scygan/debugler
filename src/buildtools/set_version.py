import sys
import os
import shutil


if len(sys.argv) < 2:
    print "not enough arhuments"
    exit(1)

shutil.move("../DGLCommon/version.cpp", "../DGLCommon/version.cpp.old")
outFile = open("../DGLCommon/version.cpp", "w")
with open("../DGLCommon/version.cpp.old", "r") as inFile:
    for line in inFile:
        if line.startswith("#define DGL_VERSION"): 
            line = "#define DGL_VERSION \"" + sys.argv[1] + "\"\n"
        outFile.write(line)
inFile.close()
outFile.close()
os.remove("../DGLCommon/version.cpp.old")


shutil.move("../buildtools/vs/auxprojects/Installer/version.wxi", "../buildtools/vs/auxprojects/Installer/version.wxi.old")
outFile = open("../buildtools/vs/auxprojects/Installer/version.wxi", "w")
with open("../buildtools/vs/auxprojects/Installer/version.wxi.old", "r") as inFile:
    for line in inFile:
        if line.startswith("    <?define version="): 
            line = "    <?define version=\"" + sys.argv[1] + "\" ?>\n"
        outFile.write(line)
inFile.close()
outFile.close()
os.remove("../buildtools/vs/auxprojects/Installer/version.wxi.old")