all: ../../dump/codegen/nonExtTypedefs.inl ../../dump/codegen/wrappers.inl ../../dump/codegen/functionList.inl ../../dump/codegen/OpenGL32.def ../../dump/codegen/enum.inl 

../../dump/codegen/nonExtTypedefs.inl ../../dump/codegen/wrappers.inl ../../dump/codegen/functionList.inl ../../dump/codegen/OpenGL32.def ../../dump/codegen/enum.inl : codegen.py input\GL.h input\wgl.h input\glext.h input\wglext.h input\wglext-partial.h
	codegen.py

clean:
	del ../../dump/codegen/*.*
	
	
	
	

