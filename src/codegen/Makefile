all: ../../dump/codegen/nonExtTypedefs.inl ../../dump/codegen/wrappers.inl ../../dump/codegen/functionList.inl ../../dump/codegen/OpenGL32.def ../../dump/codegen/enum.inl 

../../dump/codegen/nonExtTypedefs.inl ../../dump/codegen/wrappers.inl ../../dump/codegen/exporters.inl ../../dump/codegen/exporters-ext.inl ../../dump/codegen/functionList.inl ../../dump/codegen/OpenGL32.def ../../dump/codegen/enum.inl : codegen.py input\GL\GL.h input\GL\wgl.h input\GL\glext.h input\GL\wglext.h input\EGL\egl.h input\EGL\eglext.h input\GLESv1\gl.h input\GLESv1\glext.h input\GLES2\gl2.h input\GLES2\gl2ext.h input\GLES3\gl3.h input\GL\glx.h input\GL\glxext.h
	codegen.py input ../../dump/codegen/

clean:
	del ../../dump/codegen/*.*
	
	
	
	

