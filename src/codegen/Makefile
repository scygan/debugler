outputs = ..\..\dump\codegen\entrypTypedefs.inl ..\..\dump\codegen\wrappers.inl ..\..\dump\codegen\functionList.inl ..\..\dump\codegen\entrypointEnumList.inl ..\..\dump\codegen\OpenGL32.def ..\..\dump\codegen\enum.inl ..\..\dump\codegen\exporters.inl ..\..\dump\codegen\exporters-ext.inl ..\..\dump\codegen\exporters-android.inl input\EGL\egl.h  input\EGL\eglext.h input\GL\glext.h input\GL\glx.h input\GL\glxext.h input\GL\wgl.h input\GL\wglext.h input\GLES\gl.h input\GLES\glext.h input\GLES2\gl2.h input\GLES2\gl2ext.h input\GLES3\gl3.h input\GL\gl.h

inputs = codegen.py input\gl.xml input\egl.xml input\glx.xml input\wgl.xml wgl-notrace.xml gl-android.xml input\reg.py input\genheaders.py android-gles1ext.exports android-gles2ext.exports

all: $(outputs)

$(outputs) : $(inputs)
	codegen.py input ..\..\dump\codegen\

clean:
	del ..\..\dump\codegen\*.*
	
	
	
	

