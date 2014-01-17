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

inputDir  = sys.argv[1]
khrXMLDir = inputDir + "/../khronos-xml/"
outputDir = sys.argv[2]

if not os.path.exists(outputDir):
    os.makedirs(outputDir)

nonExtTypedefs = open(outputDir + "nonExtTypedefs.inl", "w")
wrappersFile = open(outputDir + "wrappers.inl", "w")
exportersFile = open(outputDir + "exporters.inl", "w")
exportersExtFile = open(outputDir + "exporters-ext.inl", "w")
functionListFile = open(outputDir + "functionList.inl", "w")
defFile = open(outputDir + "OpenGL32.def", "w")
enumFile = open(outputDir + "enum.inl", "w")


entrypoints = dict()
enums = dict()

class Enum:
    def __init__(self):
        pass


class Entrypoint:
    def __init__(self, library, genTypeDef, skipTrace, retType, paramList, paramDeclList):
        self.libraries = library
        self.genTypeDef = genTypeDef
        self.skipTrace = skipTrace
        self.retType = retType
        self.paramList = paramList
        self.paramDeclList = paramDeclList
    
    def addLibrary(self, library):
        if library not in self.libraries:
            self.libraries.append(library);
        
    def getLibaryBitMask(self): 
        if len(self.libraries) == 0:
            print "library not defined"
            exit(1)
        ret = ""
        for lib in self.libraries: 
            if len(ret) > 0:
                ret += " || "
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
    
def parse(path, library, genNonExtTypedefs = False, skipTrace = False):
    lines = open(path, "r").readlines()
    
    #merge expressions spanning multiple lines
    expressions = []
    currentExpression = ""
    for line in lines:
        if '#' in line or ';' in line or '/*' in line or '*/' in line: 
            expressions.append(currentExpression + line.strip())
            currentExpression = ""
        else:
            currentExpression = currentExpression + line.rstrip().strip()    
        if currentExpression != "":
            currentExpression += " "
    
    for line in expressions:
        enumMatch = re.match("^#define GL([a-zA-Z0-9_]*) (.*)0x(.*)$", line)
        if enumMatch and not "_LINE_BIT" in enumMatch.group(1):
            print >> enumFile, "#ifdef GL" + enumMatch.group(1)
            print >> enumFile, "    ENUM_LIST_ELEMENT(GL" + enumMatch.group(1) + ")"
            print >> enumFile, "#endif"
        coarseFunctionMatch = re.match("^([a-zA-Z0-9_]*) (.*? (?:\*)?)(WINAPI|APIENTRY|EGLAPIENTRY|GL_APIENTRY|) ?((?:w?e?gl)[a-zA-Z0-9]*) ?\((.*)\)(.*)$", line)
        if coarseFunctionMatch: 
            #print coarseFunctionMatch.groups()
            #print line
            retType = coarseFunctionMatch.group(2).strip()
            entryPointName = coarseFunctionMatch.group(4)
            #print entryPointName
            entrypointParamsStr = coarseFunctionMatch.group(5)
            entrypointParams = entrypointParamsStr.split(",")
            paramNames = []            
            paramDeclList = []
                    
            implicitParamCount = 0
            if entrypointParamsStr.strip() == "VOID" or entrypointParamsStr.strip() == "void":
                paramDeclList = [ entrypointParamsStr ]
            else:
                for param in entrypointParams:
                    #print param
                    paramMatch = re.match("^[ ]*(const|CONST|)[ ]*(struct|)[ ]*((?:unsigned )?[a-zA-Z0-9_]*)[ ]*(\*?)[ ]*(const|CONST|)[ ]*(\*?)[ ]*([a-zA-Z0-9_]*)(\[.*\])? *$", param)
                    #print paramMatch.groups()
                    paramName = paramMatch.group(7)
                    
                    if paramName == "":
                        paramName = "unnamed" + str(implicitParamCount)
                        implicitParamCount += 1
                    
                    paramNames.append(paramName)
                    
                    paramDecl = ""
                    if paramMatch.group(1):
                        paramDecl += paramMatch.group(1) + " "
                    if paramMatch.group(2):
                        paramDecl += paramMatch.group(2) + " "
                    paramDecl += paramMatch.group(3) + " "
                    if paramMatch.group(4):
                        paramDecl += paramMatch.group(4) + " "
                    if paramMatch.group(5):
                        paramDecl += paramMatch.group(5)
                    if paramMatch.group(6):
                        paramDecl += paramMatch.group(6)
                    paramDecl += paramName
                    if paramMatch.group(8):
                        paramDecl += paramMatch.group(8)
                    paramDeclList.append(paramDecl)
                
            if entryPointName in entrypoints:
                if not (library in entrypoints[entryPointName].libraries):
                    entrypoints[entryPointName].addLibrary(library)
                continue #no further processing of entrypoint
            else:
                entrypoints[entryPointName] = Entrypoint(library, genNonExtTypedefs, skipTrace, retType, paramNames, paramDeclList)
                if entryPointName == "glXGetProcAddressARB":
                    #it would be very hard to parse glXGetProcAddress from glx.h (there is func ret type in definition).
                    #So for simplicity assume glXGetProcAddress is the same as glXGetProcAddressARB
                    entrypoints["glXGetProcAddress"] = Entrypoint(library, genNonExtTypedefs, skipTrace, retType, paramNames, paramDeclList)

                    
def parseXML(path): 
    root = etree.parse(path).getroot()
    
    #gather all entrypoints
    for enumsElement in root.iter("enums"):
        for enumElement in enumsElement.iter("enum"):
            if enumElement.get("name") == None:
                print "No entrypoint name"
                print etree.tostring(enumElement)
                exit(1)
            enums[enumElement.get("name")] = Enum()
       
    for commandsElement in root.iter("commands"):
        for commandElement in commandsElement.iter("command"):
            entryPointName = commandElement.find("proto").find("name").text
           
            prototype = commandElement.find("proto")
            
            retType = ""
            if prototype.find("ptype") == None:
                retType = prototype.text
            else:
                retType = prototype.find("ptype").text
            
           
            paramNames = []            
            paramDeclList = []
            
            for paramElement in commandElement.iter("param"):
                paramNames.append(paramElement.find("name").text)
                if paramElement.find("ptype") == None:
                    type = paramElement.text
                else:
                    type = paramElement.find("ptype").text
                paramDeclList.append(type + " " + paramElement.find("name").text)
            
            genNonExtTypedefs = "TODO"
            skipTrace = "TODO"  #only wgl-notrace.h
            
            if entryPointName in entrypoints:
                print "Entrypoint already defined"
                exit(1)
            else:
                entrypoints[entryPointName] = Entrypoint([], genNonExtTypedefs, skipTrace, retType, paramNames, paramDeclList)

    
    gles2onlyPat = re.compile('2\.[0-9]')
    gles3onlyPat = re.compile('3\.[0-9]')
    gl11and10Match = re.compile('1\.[0-1]')
    gl12andLaterMatch = re.compile('1\.[2-9]|[234]\.[0-9]')
                
    for featureElement in root.iter("feature"):
        api = featureElement.get("api")
        version = featureElement.get("number")
       
        if api == "gl":
            if gl11and10Match.match(version):
                library = "LIBRARY_GL"
            elif gl12andLaterMatch.match(version):
                library = "LIBRARY_GL_EXT"
            else:
                print "Unspported version: " + version
                exit(1)
        elif api == "gles1":
            library = "LIBRARY_ES1"
        elif api == "gles2":
            if gles2onlyPat.match(version):
                library = "LIBRARY_ES2"
            elif gles3onlyPat.match(version):
                library = "LIBRARY_ES3"
            else:
                print "Unspported version: " + version
        else:
            print "Unspported api: " + api
            exit(1)
        
        for requireElement in featureElement.iter("require"):
            for commandElement in requireElement.iter("command"):
                entrypoints[commandElement.get("name")].addLibrary(library)
        
        for extensionsElement in root.iter("extensions"):
            for extensionElement in extensionsElement.iter("extension"):
                apis = extensionElement.get("supported")
                
                for api in apis.split("|"):
                    if api == "gl" or api == "glcore":
                        library = "LIBRARY_GL_EXT"
                    elif api == "gles1":
                        library = "LIBRARY_ES1_EXT"
                    elif api == "gles2":
                        library = "LIBRARY_ES2_EXT"
                    else:
                        print "Unspported api: " + api
                        exit(1)
                     
                    for requireElement in extensionElement.iter("require"):
                            for commandElement in requireElement.iter("command"):
                                entrypoints[commandElement.get("name")].addLibrary(library)
        
print >> defFile, "EXPORTS"

#currendDir = os.getcwd();
#os.chdir(khrXMLDir)
#p = subprocess.Popen(sys.executable + " genheaders.py", stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#out, error = p.communicate()
#os.chdir(currendDir)
#print out
#print error

#if p.returncode != 0:
    #print "Khronos header generation failed: " + str(p.returncode)
#    exit(1)
    
parseXML(khrXMLDir + os.sep + "gl.xml")
#parseXML(khrXMLDir + os.sep + "egl.xml")
#parseXML(khrXMLDir + os.sep + "wgl.xml")
#parseXML(khrXMLDir + os.sep + "glx.xml")

#parse(inputDir + "/GL/GL.h", "LIBRARY_GL", True)
#parse(inputDir + "/GL/glext.h", "LIBRARY_GL_EXT")

#parse(inputDir + "/GLESv1/gl.h", "LIBRARY_ES1", True)
#parse(inputDir + "/GLESv1/glext.h", "LIBRARY_ES1_EXT")
#parse(inputDir + "/GLES2/gl2.h", "LIBRARY_ES2", True)
#parse(inputDir + "/GLES2/gl2ext.h", "LIBRARY_ES2_EXT")
#parse(inputDir + "/GLES3/gl3.h", "LIBRARY_ES3", True)

#parse(inputDir + "/GL/wgl.h", "LIBRARY_WGL", True)
#parse(inputDir + "/GL/wgl-notrace.h", "LIBRARY_WGL", True, True)
#parse(inputDir + "/GL/wglext.h", "LIBRARY_WGL_EXT")

#parse(inputDir + "/EGL/egl.h", "LIBRARY_EGL", True)
#parse(inputDir + "/EGL/eglext.h", "LIBRARY_EGL_EXT")

#parse(inputDir + "/GL/glx.h", "LIBRARY_GLX", True)
#parse(inputDir + "/GL/glxext.h", "LIBRARY_GLX_EXT")

blacklist = ["glXAssociateDMPbufferSGIX", "glXCreateGLXVideoSourceSGIX", "glXDestroyGLXVideoSourceSGIX" ]

#writeout files:

for name, enum in sorted(enums.items()):
    if not "_LINE_BIT" in name:  #TODO: what about _LINE_BIT stuff?
        print >> enumFile, "#ifdef " + name
        print >> enumFile, "    ENUM_LIST_ELEMENT(" + name + ")"
        print >> enumFile, "#endif"

for name, entrypoint in sorted(entrypoints.items()):
    if name in blacklist:
        continue;

#list of entrypoints
    entrypointPtrType = "PFN" + name.upper() + "PROC"
    print >> functionListFile, entrypoint.getLibraryIfdef()
    print >> functionListFile, "    FUNC_LIST_ELEM_SUPPORTED(" + name + ", " + entrypointPtrType + ", " + entrypoint.getLibaryBitMask() + ")"
    print >> functionListFile,"#else"
    print >> functionListFile, "    FUNC_LIST_ELEM_NOT_SUPPORTED(" + name + ", " + entrypointPtrType + ", " + entrypoint.getLibaryBitMask() + ")"
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

#entrypoint wrappers

    print >> wrappersFile, entrypoint.getLibraryIfdef()
    print >> wrappersFile, "extern \"C\" " + entrypoint.retType + " APIENTRY " + name + "_Wrapper(" + listToString(entrypoint.paramDeclList) + ") {"
    
    #WA for <internalFormat> in glTexImage, glTextureImage
    # - treats internalFormat as GLenum instead of GLint, so it can be nicely displayed.
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
                entrypoint.paramDeclList[internalFormatIdx] = entrypoint.paramDeclList[internalFormatIdx].replace("GLint", "GLenum")
            else:
                raise Exception(name + '\'s 3rd parameter assumed GLuint or GLenum, got ' + entrypoint.paramDeclList[internalFormatIdx] + ', bailing out')
        else:
            raise Exception(name + '\'s 3rd parameter assumed internalFormat, got ' + entrypoint.paramDeclList[internalFormatIdx] + ', bailing out')
            
    
    if not entrypoint.skipTrace:
        cookie = "    DGLWrapperCookie cookie( " + name + "_Call"
        i = 0
        while i < len(entrypoint.paramList):
            cookie = cookie + ", "
            if "GLenum" in entrypoint.paramDeclList[i] and not "*" in entrypoint.paramDeclList[i]:
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
    if entrypoint.genTypeDef:
        print >> nonExtTypedefs, entrypoint.getLibraryIfdef()
        print >> nonExtTypedefs, "typedef " + entrypoint.retType + " (APIENTRYP " + entrypointPtrType + ")(" + listToString(entrypoint.paramDeclList) + ");"
        print >> nonExtTypedefs, "#else"
        print >> nonExtTypedefs, "typedef void * " +  entrypointPtrType + ";"
        print >> nonExtTypedefs, "#endif"
        
#def file for DLL symbols export
    if "LIBRARY_GL" in entrypoint.libraries  or "LIBRARY_WGL" in entrypoint.libraries:
        print >> defFile, "  " + name

