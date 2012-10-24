#!/usr/bin/env python
import os
import re

outputDir = "../../dump/codegen/"

if not os.path.exists(outputDir):
    os.makedirs(outputDir)

nonExtTypedefs = open(outputDir + "nonExtTypedefs.inl", "w")
wrappersFile = open(outputDir + "wrappers.inl", "w")
pointersFile = open(outputDir + "pointers.inl", "w")
functionListFile = open(outputDir + "functionList.inl", "w")
defFile = open(outputDir + "OpenGL32.def", "w")


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
			functionAttrNames = ""
			functionNamedAttrList = "";
			
			implicitAttributeNameCount = 0
			if functionAttrList == "VOID" or functionAttrList == "void":
				functionNamedAttrList = functionAttrList
			else:
				for attribute in functionAttrs:
					attributeMatch = re.match("^[ ]*(const|CONST|)[ ]*(struct|)[ ]*([a-zA-Z0-9_]*)[ ]*(\*?)[ ]*(const|CONST|)[ ]*(\*?)[ ]*([a-zA-Z0-9_]*)$", attribute)
					print attributeMatch.groups()
					attributeName = attributeMatch.group(7)
					
					if attributeName == "":
						attributeName = "unnamed" + str(implicitAttributeNameCount)
						implicitAttributeNameCount += 1
					
					if functionAttrNames != "":
						functionAttrNames = functionAttrNames + ", "
					functionAttrNames = functionAttrNames + attributeName
					
					if functionNamedAttrList != "":
						functionNamedAttrList += ", "
					
					if attributeMatch.group(1):
						functionNamedAttrList += attributeMatch.group(1) + " "
					if attributeMatch.group(2):
						functionNamedAttrList += attributeMatch.group(2) + " "
					functionNamedAttrList += attributeMatch.group(3) + " "
					if attributeMatch.group(4):
						functionNamedAttrList += attributeMatch.group(4) + " "
					if attributeMatch.group(5):
						functionNamedAttrList += attributeMatch.group(5)
					if attributeMatch.group(6):
						functionNamedAttrList += attributeMatch.group(6)
					functionNamedAttrList += attributeName

			functionPFNType = "PFN" + functionName.upper() + "PROC"						
			print >> functionListFile, "FUNCTION_LIST_ELEMENT(" + functionName + ", " + functionPFNType + ")"
			print >> pointersFile, "PTR_PREFIX " + functionPFNType + " POINTER(" + functionName  + ");"

			print >> wrappersFile, "extern \"C\" DGLWRAPPER_API " + functionRetType + " APIENTRY " + functionName + "(" + functionNamedAttrList + ") {"
			print >> wrappersFile, "    assert(POINTER(" + functionName + "));"
			print >> wrappersFile, "    RetValue retVal = g_Tracers[" + functionName + "_Call]->Pre(" + functionName + "_Call);"
			print >> wrappersFile, "    if (!retVal.isSet()) {"
			if functionRetType != "void":
				print >> wrappersFile, "    	retVal = DIRECT_CALL(" + functionName + ")(" + functionAttrNames + ");"
			else:
				print >> wrappersFile, "    	DIRECT_CALL(" + functionName + ")(" + functionAttrNames + ");"			
			print >> wrappersFile, "    }"
			print >> wrappersFile, "    g_Tracers[" + functionName + "_Call]->Post(" + functionName + "_Call);"
			if functionRetType != "void":
				print >> wrappersFile, "    return (" + functionRetType + ")retVal;"
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

#tempFile = open("input/temp.h", "r").readlines()
#parse(tempFile)
