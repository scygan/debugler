#!/usr/bin/env python
# Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#



import os
import re
import sys
from sets import Set
import subprocess
from lxml import etree

inputDir  = sys.argv[1] + os.sep
outputDir = sys.argv[2] + os.sep

if not os.path.exists(outputDir):
    os.makedirs(outputDir)

entrypTypedefs = open(outputDir + "entrypTypedefs.inl", "w")
wrappersFile = open(outputDir + "wrappers.inl", "w")
exportersFile = open(outputDir + "exporters.inl", "w")
exportersExtFile = open(outputDir + "exporters-ext.inl", "w")
exportersAndroidFile = open(outputDir + "exporters-android.inl", "w")
functionListFile = open(outputDir + "functionList.inl", "w")
defFile = open(outputDir + "OpenGL32.def", "w")
enumFile = open(outputDir + "enum.inl", "w")

gles2onlyPat = re.compile('2\.[0-9]')
gles3onlyPat = re.compile('3\.[0-9]')
gl11and10Match = re.compile('1\.[0-1]')
gl12andLaterMatch = re.compile('1\.[2-9]|[234]\.[0-9]')

# These are "mandatory" OpenGL ES 1 extensions, to
# be LIBRARY_ES1 (not LIBRARY_ES1_EXT).
es1CoreList = [
    'GL_OES_read_format',
    'GL_OES_compressed_paletted_texture',
    'GL_OES_point_size_array',
    'GL_OES_point_sprite'
]


entrypoints = dict()
enums = dict()

class Enum:
    def __init__(self, value):
        self.value = value


class Entrypoint:
    def __init__(self, library, skipTrace, retType, paramList, paramDeclList):
        self.libraries = library
        self.skipTrace = skipTrace
        self.retType = retType
        self.paramList = paramList
        self.paramDeclList = paramDeclList
        self.forceEnumIndices = []
    
    def addLibrary(self, library):
        if library not in self.libraries:
            self.libraries.append(library);
        
        if "LIBRARY_GL_EXT" in self.libraries and "LIBRARY_GL" in self.libraries:
            #this happens, when entrypoint is introduced in GL 1.1, removed in 3.2 and re-introduced later
            #we don't care for this, we cannot mark is as ext - PFN...PROC defintion is still required.
            self.libraries.remove("LIBRARY_GL_EXT")
        
    def getLibaryBitMask(self): 
        if len(self.libraries) == 0:
            print "library not defined"
            exit(1)
        ret = ""
        for lib in self.libraries: 
            if len(ret) > 0:
                ret += " | "
            ret += lib.strip()
        return ret
        
    def getLibraryIfdef(self):
        if len(self.libraries) == 0:
            print "library not defined"
            exit(1)
        ret = "#if "
        first = True
        for lib in self.libraries: 
            if not first:
                ret += " || "
            first = False
            ret += "defined(HAVE_" + lib.strip() + ")"
        return ret
        
def listToString(list):
    str = "";
    for elem in list:
        if str != "":
            str += ", "
        str += elem
    return str        
        
def isPointer(type):
    pointers = [ "*", "PROC", "LPCSTR", "HGLRC", "HDC", "LPPIXELFORMATDESCRIPTOR", "LPLAYERPLANEDESCRIPTOR", "LPGLYPHMETRICSFLOAT", "GLsync" ]
    if any(pointer in type for pointer in pointers):
        return True
    return False
    

def libraryFromApiXML(api, isExtension, version = ""):
    if isExtension:
        if api == "gl" or api == "glcore":
            return "LIBRARY_GL_EXT"
        elif api == "gles1":
            return "LIBRARY_ES1_EXT"
        elif api == "gles2":
            return "LIBRARY_ES2_EXT"
        elif api == "egl":
            return "LIBRARY_EGL_EXT"
        elif api == "wgl":
            return "LIBRARY_WGL_EXT"
        elif api == "glx":
            return "LIBRARY_GLX_EXT"
        else:
            print "Unspported api: " + api
            exit(1)
    else:
        if api == "gl":
            if gl11and10Match.match(version):
                return "LIBRARY_GL"
            elif gl12andLaterMatch.match(version):
                return "LIBRARY_GL_EXT"
            else:
                print "Unspported version: " + version
                exit(1)
        elif api == "gles1":
            return "LIBRARY_ES1"
        elif api == "gles2":
            if gles2onlyPat.match(version):
                return "LIBRARY_ES2"
            elif gles3onlyPat.match(version):
                return "LIBRARY_ES3"
            else:
                print "Unspported version: " + version
        elif api == "egl":
            return "LIBRARY_EGL"
        elif api == "wgl":
            return "LIBRARY_WGL"
        elif api == "glx":
            return "LIBRARY_GLX"
        else:
            print "Unspported api: " + api
            exit(1)
    

def getCTypeFromXML(element):
    if element.text != None:
        retType = element.text
    else:
        retType = ""
    if element.find("ptype") != None:
        retType += element.find("ptype").text
        retType += element.find("ptype").tail
    return retType.strip()

    
def parseXML(path, skipTrace = False): 
    root = etree.parse(path).getroot()
    
    #gather all entrypoints
    for enumsElement in root.iter("enums"):
        for enumElement in enumsElement.iter("enum"):
            name = enumElement.get("name")
            value = enumElement.get("value")
            if name == None or value == None:
                print "No entrypoint name/value"
                print etree.tostring(enumElement)
                exit(1)
            enums[name] = Enum(value)
       
    for commandsElement in root.iter("commands"):
        for commandElement in commandsElement.iter("command"):
            entryPointName = commandElement.find("proto").find("name").text
           
            prototype = commandElement.find("proto")
            
            retType = getCTypeFromXML(prototype)
          
            paramNames = []            
            paramDeclList = []
            
            for paramElement in commandElement.iter("param"):
                name = paramElement.find("name").text
                if name == "near" or name == "far": 
                    #these are reserved keywords in C
                    name = "_" + name
                paramNames.append(name)
                type = getCTypeFromXML(paramElement)
                paramDeclList.append(type + " " + name)
                        
            if entryPointName in entrypoints:
                print "Entrypoint already defined"
                exit(1)
            else:
                entrypoints[entryPointName] = Entrypoint([], skipTrace, retType, paramNames, paramDeclList)

    

                
    for featureElement in root.iter("feature"):
        api = featureElement.get("api")
        version = featureElement.get("number")
       
        library = libraryFromApiXML(api, False, version)
        
        for requireElement in featureElement.iter("require"):
            for commandElement in requireElement.iter("command"):
                entrypoints[commandElement.get("name")].addLibrary(library)
        
    
    for extensionsElement in root.iter("extensions"):
        for extensionElement in extensionsElement.iter("extension"):
            apis = extensionElement.get("supported")
            
            for api in apis.split("|"):
                library = libraryFromApiXML(api, True)
                
                #there are three "mandatory" exts in es1, that are defined in GLES/gl.h
                forceLibEs1 = (extensionElement.get("name") in es1CoreList and api == "gles1")
                
                for requireElement in extensionElement.iter("require"):
                    if requireElement.get("api") != None and requireElement.get("api") != api:
                        #skip incompatible <require> element. will handle when doing proper api.
                        continue
                    for commandElement in requireElement.iter("command"):
                        if forceLibEs1:
                            entrypoints[commandElement.get("name")].addLibrary("LIBRARY_ES1")
                        else:
                            entrypoints[commandElement.get("name")].addLibrary(library)
        
print >> defFile, "EXPORTS"

headersToGenerate = dict()
headersToGenerate["GL/glext.h"] = "gl.xml"
headersToGenerate["GLES/gl.h"] = "gl.xml"
headersToGenerate["GLES/glext.h"] = "gl.xml"
headersToGenerate["GLES2/gl2.h"] = "gl.xml"
headersToGenerate["GLES2/gl2ext.h"] = "gl.xml"
headersToGenerate["GLES3/gl3.h"] = "gl.xml"

headersToGenerate["EGL/egl.h"] = "egl.xml"
headersToGenerate["EGL/eglext.h"] = "egl.xml"

headersToGenerate["GL/wgl.h"] = "wgl.xml"
headersToGenerate["GL/wglext.h"] = "wgl.xml"

headersToGenerate["GL/glx.h"] = "glx.xml"
headersToGenerate["GL/glxext.h"] = "glx.xml"

for name, registry in headersToGenerate.items():
    currendDir = os.getcwd();
    os.chdir(inputDir)
    p = subprocess.Popen([sys.executable, "genheaders.py", "-registry", registry, name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, error = p.communicate()
    os.chdir(currendDir)
    print out
    print error

    if p.returncode != 0:
        print "Khronos header generation failed: " + str(p.returncode)
        exit(1)



    
parseXML(inputDir + "gl.xml")
parseXML(inputDir + "egl.xml")
parseXML(inputDir + "wgl.xml")
parseXML(inputDir + "glx.xml")


#----- WORKAROUNDS START -------

#System: ALL
#API: GL, GLES
#
# Change <internalFormat> of all *TexImage* calls to GLenum, as GLint cannot be nicely printed.
    #WA for <internalFormat> in glTexImage, glTextureImage
    # - treats internalFormat as GLenum instead of GLint, so it can be nicely displayed.
for name, entrypoint in sorted(entrypoints.items()):
    if name.startswith("glTexImage") or name.startswith("glTextureImage"):
        if "MultisampleCoverageNV" in name:
            internalFormatIdx = 3;
        else:
            internalFormatIdx = 2;        
        if name.startswith("glTextureImage"):
            internalFormatIdx += 1
        if entrypoint.paramList[internalFormatIdx].lower() == "internalformat":
            if "GLenum" in entrypoint.paramDeclList[internalFormatIdx]:
                pass#this is ok
            elif "GLint" in entrypoint.paramDeclList[internalFormatIdx]:
                #replace
                entrypoints[name].forceEnumIndices.append(internalFormatIdx)
            else:
                raise Exception(name + '\'s 3rd parameter assumed GLuint or GLenum, got ' + entrypoint.paramDeclList[internalFormatIdx] + ', bailing out')
        else:
            raise Exception(name + '\'s 3rd parameter assumed internalFormat, got ' + entrypoint.paramDeclList[internalFormatIdx] + ', bailing out')


#System: ALL
#
#This function exist in khronox xml registry, but is not associated with any feature:
if len(entrypoints["wglGetDefaultProcAddress"].libraries) > 0:
    print "Fix me - remove WA"
    exit(1)
entrypoints["wglGetDefaultProcAddress"].addLibrary("LIBRARY_WGL")


#System: Windows
#API: WGL, undocumented, called by GDI
#
#These are required, if we want to act as "opengl32.dll". However we do not trace them.
parseXML(inputDir + ".." + os.sep + "wgl-notrace.xml", True)


#System: Windows
#API: gdi32.dll
#Registry enlists these functions as WGL, however they present only in gdi32.dll
entrypoints["GetPixelFormat"].libraries =            ["LIBRARY_WINGDI"];
entrypoints["ChoosePixelFormat"].libraries =         ["LIBRARY_WINGDI"];
entrypoints["DescribePixelFormat"].libraries =       ["LIBRARY_WINGDI"];
entrypoints["GetEnhMetaFilePixelFormat"].libraries = ["LIBRARY_WINGDI"];
entrypoints["GetPixelFormat"].libraries =            ["LIBRARY_WINGDI"];
entrypoints["SetPixelFormat"].libraries =            ["LIBRARY_WINGDI"];
entrypoints["SwapBuffers"].libraries =               ["LIBRARY_WINGDI"];


#System: Android
#API: GLES1, undocumented *Bounds entrypoins
#These undocumented symbols exported by libGLESv1.so, called sometimes by JNI
parseXML(inputDir + ".." + os.sep + "gl-android.xml", True)

#System: Android
#API: GLES1
#Some extensions are exported in libGLESv1.so
androidGLES1Exports = open( inputDir + ".." + os.sep + "android-gles1ext.exports", "r" )
for entryp in androidGLES1Exports:
    if "LIBRARY_ES1" not in entrypoints[entryp.strip()].libraries:
        entrypoints[entryp.strip()].addLibrary("LIBRARY_ES1_ANDROID")

#System: Android
#API: GLES2
#Some extensions are exported in libGLESv2.so
androidGLES2Exports = open( inputDir + ".." + os.sep + "android-gles2ext.exports", "r" )
for entryp in androidGLES2Exports:
    if "LIBRARY_ES2" not in entrypoints[entryp.strip()].libraries:
        entrypoints[entryp.strip()].addLibrary("LIBRARY_ES2_ANDROID")


#System: Linux/X11
#API: GLX EXT
#These rare functions require some external headers. 
blacklist = ["glXAssociateDMPbufferSGIX", "glXCreateGLXVideoSourceSGIX", "glXDestroyGLXVideoSourceSGIX" ]


#----- WORKAROUNDS END -------


#writeout files:

for name, enum in sorted(enums.items()):
    if not "_LINE_BIT" in name:  #TODO: what about _LINE_BIT stuff?
        print >> enumFile, "ENUM_LIST_ELEMENT(" + name + ","  + enum.value + ")"

for name, entrypoint in sorted(entrypoints.items()):
    if name in blacklist:
        continue;

#list of entrypoints
    entrypointPtrType = "DGL_PFN" + name.upper() + "PROC"
    print >> functionListFile, entrypoint.getLibraryIfdef()
    print >> functionListFile, "    FUNC_LIST_ELEM_SUPPORTED(" + name + ", " + entrypointPtrType + ", " + entrypoint.getLibaryBitMask() + ")"
    print >> functionListFile,"#else"
    print >> functionListFile, "    FUNC_LIST_ELEM_NOT_SUPPORTED(" + name + ", " + entrypointPtrType + ", LIBRARY_NONE)"
    print >> functionListFile,"#endif"
    
#entrypoint exporters
    coreLib = False
    for coreLib1 in entrypoint.libraries:
        for coreLib2 in ["LIBRARY_WGL", "LIBRARY_GLX", "LIBRARY_EGL", "LIBRARY_GL", "LIBRARY_ES1", "LIBRARY_ES2", "LIBRARY_ES3" ]:
            if coreLib1.strip() == coreLib2.strip():
                coreLib = True
                
    if coreLib:
        print >> exportersFile, entrypoint.getLibraryIfdef()
        print >> exportersFile, "extern \"C\" DGLWRAPPER_API " + entrypoint.retType + " APIENTRY " + name + "(" + listToString(entrypoint.paramDeclList) + ") {"
        print >> exportersFile, "        return " + name + "_Wrapper(" + listToString(entrypoint.paramList) + ");"        
        print >> exportersFile, "}"
        print >> exportersFile, "#endif"
    else:
        print >> exportersExtFile, entrypoint.getLibraryIfdef()
        print >> exportersExtFile, "extern \"C\" DGLWRAPPER_API " + entrypoint.retType + " APIENTRY " + name + "(" + listToString(entrypoint.paramDeclList) + ") {"
        print >> exportersExtFile, "        return " + name + "_Wrapper(" + listToString(entrypoint.paramList) + ");"        
        print >> exportersExtFile, "}"
        print >> exportersExtFile, "#endif"
        
    androidLib = False
    for androidLib1 in entrypoint.libraries:
        for androidLib2 in ["LIBRARY_ES1_ANDROID", "LIBRARY_ES2_ANDROID" ]:
            if androidLib1.strip() == androidLib2.strip():
                androidLib = True
    if androidLib:
        print >> exportersAndroidFile, entrypoint.getLibraryIfdef()
        print >> exportersAndroidFile, "extern \"C\" DGLWRAPPER_API " + entrypoint.retType + " APIENTRY " + name + "(" + listToString(entrypoint.paramDeclList) + ") {"
        print >> exportersAndroidFile, "        return " + name + "_Wrapper(" + listToString(entrypoint.paramList) + ");"        
        print >> exportersAndroidFile, "}"
        print >> exportersAndroidFile, "#endif"
        
#entrypoint wrappers

    print >> wrappersFile, entrypoint.getLibraryIfdef()
    print >> wrappersFile, "extern \"C\" " + entrypoint.retType + " APIENTRY " + name + "_Wrapper(" + listToString(entrypoint.paramDeclList) + ") {"
    
    if not entrypoint.skipTrace:
        cookie = "    DGLWrapperCookie cookie( " + name + "_Call"
        i = 0
        while i < len(entrypoint.paramList):
            cookie = cookie + ", "
            if ("GLenum" in entrypoint.paramDeclList[i] and not "*" in entrypoint.paramDeclList[i]) or i in entrypoint.forceEnumIndices:
                cookie = cookie + "GLenumWrap(" + entrypoint.paramList[i] + ")"
            else:
                cookie = cookie + entrypoint.paramList[i]
            i+=1
                
        print >> wrappersFile, cookie + " );"
    
        print >> wrappersFile, "    if (!cookie.retVal.isSet()) {"
        print >> wrappersFile, "        assert(POINTER(" + name + "));"
        if entrypoint.retType.lower() != "void":
            print >> wrappersFile, "        cookie.retVal = DIRECT_CALL(" + name + ")(" + listToString(entrypoint.paramList) + ");"
        else:
            print >> wrappersFile, "        DIRECT_CALL(" + name + ")(" + listToString(entrypoint.paramList) + ");"            
        print >> wrappersFile, "    }"
        
        if entrypoint.retType.lower() != "void":
            print >> wrappersFile, "    " + entrypoint.retType + " tmp;  cookie.retVal.get(tmp); return tmp;"
    else:
        print >> wrappersFile, "    //this function is not traced"
        if entrypoint.retType.lower() != "void":
            print >> wrappersFile, "    return DIRECT_CALL(" + name + ")(" + listToString(entrypoint.paramList) + ");"
        else:
            print >> wrappersFile, "    DIRECT_CALL(" + name + ")(" + listToString(entrypoint.paramList) + ");"

    print >> wrappersFile, "}"
    print >> wrappersFile, "#endif"
    
    
#additional PFN definitions
    print >> entrypTypedefs, entrypoint.getLibraryIfdef()
    print >> entrypTypedefs, "typedef " + entrypoint.retType + " (APIENTRYP " + entrypointPtrType + ")(" + listToString(entrypoint.paramDeclList) + ");"
    print >> entrypTypedefs, "#else"
    print >> entrypTypedefs, "typedef void * " +  entrypointPtrType + ";"
    print >> entrypTypedefs, "#endif"
        
#def file for DLL symbols export
    if "LIBRARY_GL" in entrypoint.libraries  or "LIBRARY_WGL" in entrypoint.libraries:
        print >> defFile, "  " + name

