outputs = ..\..\dump\codegen\entrypTypedefs.inl ..\..\dump\codegen\wrappers.inl ..\..\dump\codegen\functionList.inl ..\..\dump\codegen\entrypointEnumList.inl ..\..\dump\codegen\OpenGL32.def ..\..\dump\codegen\enum.inl ..\..\dump\codegen\exporters.inl ..\..\dump\codegen\exporters-ext.inl ..\..\dump\codegen\exporters-android.inl ..\..\dump\codegen\EGL\egl.h  ..\..\dump\codegen\EGL\eglext.h ..\..\dump\codegen\GL\gl.h ..\..\dump\codegen\GL\glext.h ..\..\dump\codegen\GL\glx.h ..\..\dump\codegen\GL\glxext.h ..\..\dump\codegen\GL\wgl.h ..\..\dump\codegen\GL\wglext.h ..\..\dump\codegen\GLES\gl.h ..\..\dump\codegen\GLES\glext.h ..\..\dump\codegen\GLES2\gl2.h ..\..\dump\codegen\GLES2\gl2ext.h ..\..\dump\codegen\GLES3\gl3.h 

inputs = codegen.py input\gl.xml input\egl.xml input\glx.xml input\wgl.xml wgl-notrace.xml gl-android.xml input\reg.py input\genheaders.py android-gles1ext.exports android-gles2ext.exports

all: $(outputs)

$(outputs) : $(inputs)
	codegen.py input ..\..\dump\codegen\

clean:
	del ..\..\dump\codegen\*.*
	
	
	
	

