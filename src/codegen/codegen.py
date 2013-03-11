#!/usr/bin/env python
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
functionListFile = open(outputDir + "functionList.inl", "w")
defFile = open(outputDir + "OpenGL32.def", "w")
enumFile = open(outputDir + "enum.inl", "w")


functions = dict()

def isPointer(type):
	pointers = [ "*", "PROC", "LPCSTR", "HGLRC", "HDC", "LPPIXELFORMATDESCRIPTOR", "LPLAYERPLANEDESCRIPTOR", "LPGLYPHMETRICSFLOAT", "GLsync" ]
	if any(pointer in type for pointer in pointers):
		return True
	return False
	
def parse(file, library, genNonExtTypedefs = False, skipTrace = False):
	for line in file:
		enumMatch = re.match("^#define GL([a-zA-Z0-9_]*) (.*)0x(.*)$", line)
		if enumMatch and not "_LINE_BIT" in enumMatch.group(1):
			print >> enumFile, "#ifdef GL" + enumMatch.group(1)
			print >> enumFile, "	ENUM_LIST_ELEMENT(GL" + enumMatch.group(1) + ")"
			print >> enumFile, "#endif"
		coarseFunctionMatch = re.match("^([a-zA-Z0-9]*) (.*) (WINAPI|APIENTRY|EGLAPIENTRY) ([a-zA-Z0-9]*) ?\((.*)\)(.*)$", line)
		if coarseFunctionMatch: 
			#print coarseFunctionMatch.groups()
			functionRetType = coarseFunctionMatch.group(2)
			functionName = coarseFunctionMatch.group(4)
			print functionName
			functionAttrList = coarseFunctionMatch.group(5)
			functionAttrs = functionAttrList.split(",")
			functionAttrNamesList = []			
			functionAttrDeclList = []
					
			implicitAttributeNameCount = 0
			if functionAttrList == "VOID" or functionAttrList == "void":
				functionAttrDeclList = [ functionAttrList ]
			else:
				for attribute in functionAttrs:
					attributeMatch = re.match("^[ ]*(const|CONST|)[ ]*(struct|)[ ]*([a-zA-Z0-9_]*)[ ]*(\*?)[ ]*(const|CONST|)[ ]*(\*?)[ ]*([a-zA-Z0-9_]*)$", attribute)
					#print attributeMatch.groups()
					attributeName = attributeMatch.group(7)
					
					if attributeName == "":
						attributeName = "unnamed" + str(implicitAttributeNameCount)
						implicitAttributeNameCount += 1
					
					functionAttrNamesList.append(attributeName)
					
					functionAttrDecl = ""
					if attributeMatch.group(1):
						functionAttrDecl += attributeMatch.group(1) + " "
					if attributeMatch.group(2):
						functionAttrDecl += attributeMatch.group(2) + " "
					functionAttrDecl += attributeMatch.group(3) + " "
					if attributeMatch.group(4):
						functionAttrDecl += attributeMatch.group(4) + " "
					if attributeMatch.group(5):
						functionAttrDecl += attributeMatch.group(5)
					if attributeMatch.group(6):
						functionAttrDecl += attributeMatch.group(6)
					functionAttrDecl += attributeName
					functionAttrDeclList.append(functionAttrDecl)
					

			functionAttrDecls = "";
			for attr in functionAttrDeclList:
				if functionAttrDecls != "":
					functionAttrDecls += ", "
				functionAttrDecls += attr
				
			functionAttrNames = ""
			for attr in functionAttrNamesList:
				if functionAttrNames != "":
					functionAttrNames = functionAttrNames + ", "
				functionAttrNames += attr

			if functionName in functions:
				functions[functionName] += " | " + library
				continue #no further processing of function
			else:
			    functions[functionName] = library

				
			functionPFNType = "PFN" + functionName.upper() + "PROC"
			print >> wrappersFile, "#ifdef HAVE_" + library
			print >> wrappersFile, "extern \"C\" DGLWRAPPER_API " + functionRetType + " APIENTRY " + functionName + "(" + functionAttrDecls + ") {"
			print >> wrappersFile, "    assert(POINTER(" + functionName + "));"
			
			if not skipTrace:
				print >> wrappersFile, "	CalledEntryPoint call( " + functionName + "_Call, " + str(len(functionAttrNamesList)) + ");"

				i = 0
				while i < len(functionAttrNamesList):
					if "GLenum" in functionAttrDeclList[i] and not "*" in functionAttrDeclList[i]:
						print >> wrappersFile, "	call << GLenumWrap(" + functionAttrNamesList[i] + ");"
					else:
						print >> wrappersFile, "	call << " + functionAttrNamesList[i] + ";"
					i+=1
			
			print >> wrappersFile, "    RetValue retVal;"
			
			if not skipTrace:
				print >> wrappersFile, "    retVal = g_Tracers[" + functionName + "_Call]->DoPre(call);"

			print >> wrappersFile, "    if (!retVal.isSet()) {"
			if functionRetType != "void":
				print >> wrappersFile, "    	retVal = DIRECT_CALL(" + functionName + ")(" + functionAttrNames + ");"
			else:
				print >> wrappersFile, "    	DIRECT_CALL(" + functionName + ")(" + functionAttrNames + ");"			
			print >> wrappersFile, "    }"
			
			if not skipTrace:
				if functionRetType != "void":
					print >> wrappersFile, "    g_Tracers[" + functionName + "_Call]->DoPost(call, retVal);"
					
				else:
					print >> wrappersFile, "    g_Tracers[" + functionName + "_Call]->DoPost(call);"
				

			if functionRetType != "void":
				print >> wrappersFile, "    " + functionRetType + " tmp;  retVal.get(tmp); return tmp;"

			print >> wrappersFile, "}"
			print >> wrappersFile, "#endif"

			if genNonExtTypedefs:
				print >> nonExtTypedefs, "#ifdef HAVE_" + library
				print >> nonExtTypedefs, "typedef " + functionRetType + " (APIENTRYP " + functionPFNType + ")(" + functionAttrList + ");"
				print >> nonExtTypedefs, "#else"
				print >> nonExtTypedefs, "typedef void * " +  functionPFNType + ";"
				print >> nonExtTypedefs, "#endif"
		
			print >> defFile, "  " + functionName

print >> defFile, "LIBRARY opengl32.dll"
print >> defFile, "EXPORTS"

			
wglFile = open(inputDir + "/wgl.h", "r").readlines()
wglextFile = open(inputDir + "/wglext-partial.h", "r").readlines()
glFile = open(inputDir + "/GL.h", "r").readlines()
glextFile = open(inputDir + "/glext.h", "r").readlines()
wglNoTraceFile = open(inputDir + "/wgl-notrace.h", "r").readlines()
eglFile = open(inputDir + "/egl.h", "r").readlines()
eglextFile = open(inputDir + "/eglext.h", "r").readlines()

parse(wglFile, "LIBRARY_WGL", True)
parse(wglextFile, "LIBRARY_WGL_EXT")
parse(glFile, "LIBRARY_GL", True)
parse(glextFile, "LIBRARY_GL_EXT")
parse(wglNoTraceFile, "LIBRARY_WGL", True, True)
parse(eglFile, "LIBRARY_EGL", True)
parse(eglextFile, "LIBRARY_EGL_EXT")



for function, library in sorted(functions.items()):
	functionPFNType = "PFN" + function.upper() + "PROC"						
	print >> functionListFile, "FUNCTION_LIST_ELEMENT(" + function + ", " + functionPFNType + ", " + library + ")"

