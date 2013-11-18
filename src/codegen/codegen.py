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

inputDir  = sys.argv[1]
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


class Entrypoint:
    def __init__(self, library, genTypeDef, skipTrace, retType, paramList, paramDeclList):
        self.libraries = library
        self.genTypeDef = genTypeDef
        self.skipTrace = skipTrace
        self.retType = retType
        self.paramList = paramList
        self.paramDeclList = paramDeclList
    
    def addLibrary(self, library):
        self.libraries += " | " + library;
    
    def getLibraryIfdef(self):
        ret = "#if "
        first = True
        for lib in self.libraries.split("|"): 
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

print >> defFile, "EXPORTS"

parse(inputDir + "/GL/GL.h", "LIBRARY_GL", True)
parse(inputDir + "/GL/glext.h", "LIBRARY_GL_EXT")

parse(inputDir + "/GLESv1/gl.h", "LIBRARY_ES1", True)
parse(inputDir + "/GLESv1/glext.h", "LIBRARY_ES1_EXT")
parse(inputDir + "/GLES2/gl2.h", "LIBRARY_ES2", True)
parse(inputDir + "/GLES2/gl2ext.h", "LIBRARY_ES2_EXT")
parse(inputDir + "/GLES3/gl3.h", "LIBRARY_ES3", True)

parse(inputDir + "/GL/wgl.h", "LIBRARY_WGL", True)
parse(inputDir + "/GL/wgl-notrace.h", "LIBRARY_WGL", True, True)
parse(inputDir + "/GL/wglext.h", "LIBRARY_WGL_EXT")

parse(inputDir + "/EGL/egl.h", "LIBRARY_EGL", True)
parse(inputDir + "/EGL/eglext.h", "LIBRARY_EGL_EXT")

parse(inputDir + "/GL/glx.h", "LIBRARY_GLX", True)
parse(inputDir + "/GL/glxext.h", "LIBRARY_GLX_EXT")

blacklist = ["glXAssociateDMPbufferSGIX", "glXCreateGLXVideoSourceSGIX", "glXDestroyGLXVideoSourceSGIX" ]

#writeout files:
for name, entrypoint in sorted(entrypoints.items()):
    if name in blacklist:
        continue;

#list of entrypoints
    entrypointPtrType = "PFN" + name.upper() + "PROC"
    print >> functionListFile, entrypoint.getLibraryIfdef()
    print >> functionListFile, "    FUNC_LIST_ELEM_SUPPORTED(" + name + ", " + entrypointPtrType + ", " + entrypoint.libraries + ")"
    print >> functionListFile,"#else"
    print >> functionListFile, "    FUNC_LIST_ELEM_NOT_SUPPORTED(" + name + ", " + entrypointPtrType + ", " + entrypoint.libraries + ")"
    print >> functionListFile,"#endif"
    
#entrypoint exporters
    coreLib = False
    for coreLib1 in entrypoint.libraries.split('|'):
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
        print >> wrappersFile, "    	assert(POINTER(" + name + "));"
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
    if "LIBRARY_GL" in entrypoint.libraries.split('|') or "LIBRARY_WGL" in entrypoint.libraries.split('|'):
        print >> defFile, "  " + name

