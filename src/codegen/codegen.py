#!/usr/bin/env python
import os
import re
from sets import Set

outputDir = "../../dump/codegen/"

if not os.path.exists(outputDir):
    os.makedirs(outputDir)

nonExtTypedefs = open(outputDir + "nonExtTypedefs.inl", "w")
wrappersFile = open(outputDir + "wrappers.inl", "w")
pointersFile = open(outputDir + "pointers.inl", "w")
functionListFile = open(outputDir + "functionList.inl", "w")
defFile = open(outputDir + "OpenGL32.def", "w")


#allTypes = Set([])

def isPointer(type):
	pointers = [ "*", "PROC", "LPCSTR", "HGLRC", "HDC", "LPPIXELFORMATDESCRIPTOR", "LPLAYERPLANEDESCRIPTOR", "LPGLYPHMETRICSFLOAT", "GLsync" ]
	if any(pointer in type for pointer in pointers):
		return True
	return False
	
def parse(file, genNonExtTypedefs = False):
	for line in file:
		coarseFunctionMatch = re.match("^([a-zA-Z0-9]*) (.*) (WINAPI|APIENTRY) ([a-zA-Z0-9]*) \((.*)\)(.*)$", line)
		if coarseFunctionMatch: 
			print coarseFunctionMatch.groups()
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
					print attributeMatch.groups()
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
					
					#allTypes.add(attributeMatch.group(3))

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
			
			
			functionPFNType = "PFN" + functionName.upper() + "PROC"						
			print >> functionListFile, "FUNCTION_LIST_ELEMENT(" + functionName + ", " + functionPFNType + ")"
			print >> pointersFile, "PTR_PREFIX " + functionPFNType + " POINTER(" + functionName  + ");"

			print >> wrappersFile, "extern \"C\" DGLWRAPPER_API " + functionRetType + " APIENTRY " + functionName + "(" + functionAttrDecls + ") {"
			print >> wrappersFile, "    assert(POINTER(" + functionName + "));"
			print >> wrappersFile, "	CalledEntryPoint call( " + functionName + "_Call, " + str(len(functionAttrNamesList)) + ");"

			for attrName in functionAttrNamesList:
				print >> wrappersFile, "	call << " + attrName + ";"
			
			print >> wrappersFile, "    RetValue retVal = g_Tracers[" + functionName + "_Call]->Pre(call);"
			print >> wrappersFile, "    if (!retVal.isSet()) {"
			if functionRetType != "void":
				print >> wrappersFile, "    	retVal = DIRECT_CALL(" + functionName + ")(" + functionAttrNames + ");"
			else:
				print >> wrappersFile, "    	DIRECT_CALL(" + functionName + ")(" + functionAttrNames + ");"			
			print >> wrappersFile, "    }"
			
			
			if functionRetType != "void":
				print >> wrappersFile, "    g_Tracers[" + functionName + "_Call]->Post(call, retVal);"
				print >> wrappersFile, "    " + functionRetType + " tmp;  retVal.get(tmp); return tmp;"
			else:
				print >> wrappersFile, "    g_Tracers[" + functionName + "_Call]->Post(call);"
			print >> wrappersFile, "}"
			
			if genNonExtTypedefs:
				print >> nonExtTypedefs, "typedef " + functionRetType + " (APIENTRYP " + functionPFNType + ")(" + functionAttrList + ");"
			
			print >> defFile, "  " + functionName

print >> defFile, "LIBRARY opengl32.dll"
print >> defFile, "EXPORTS"

			
wglFile = open("input/wgl.h", "r").readlines()
parse(wglFile)

glFile = open("input/GL.h", "r").readlines()
parse(glFile, True)

glFile = open("input/glext.h", "r").readlines()
parse(glFile)

#typeFile  = open(outputDir + "types.inl", "w")
#for type in allTypes:
#	print >> typeFile, "TYPE_LIST_ELEMENT(" + type  + ")"

#tempFile = open("input/temp.h", "r").readlines()
#parse(tempFile)
