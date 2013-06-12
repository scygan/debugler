all: ../../dump/codegen/nonExtTypedefs.inl ../../dump/codegen/wrappers.inl ../../dump/codegen/functionList.inl ../../dump/codegen/OpenGL32.def ../../dump/codegen/enum.inl 

../../dump/codegen/nonExtTypedefs.inl ../../dump/codegen/wrappers.inl ../../dump/codegen/functionList.inl ../../dump/codegen/OpenGL32.def ../../dump/codegen/enum.inl : codegen.py input\GL.h input\wgl.h input\glext.h input\wglext.h input\egl.h input\eglext.h input\gl2.h input\gl3.h input\glx.h
	codegen.py input ../../dump/codegen/

clean:
	del ../../dump/codegen/*.*
	
	
	
	

