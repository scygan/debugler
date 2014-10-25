outputs = ..\..\dump\codegen\codegen_gl_pfn_types.inl       \
          ..\..\dump\codegen\codegen_gl_enum_list.inl       \
          ..\..\dump\codegen\codegen_gl_enum_group_list.inl \
          ..\..\dump\codegen\codegen_gl_functions.inl       \
          ..\..\dump\codegen\codegen_gl_function_list.inl   \
          ..\..\dump\codegen\codegen_dgl_wrappers.inl       \
          ..\..\dump\codegen\codegen_dgl_export.inl         \
          ..\..\dump\codegen\codegen_dgl_export_ext.inl     \
          ..\..\dump\codegen\codegen_dgl_export_android.inl \
          ..\..\dump\codegen\codegen_opengl32.def           \
          ..\..\dump\codegen\EGL\egl.h                      \
          ..\..\dump\codegen\EGL\eglext.h                   \
          ..\..\dump\codegen\GL\gl.h                        \
          ..\..\dump\codegen\GL\glext.h                     \
          ..\..\dump\codegen\GL\glx.h                       \
          ..\..\dump\codegen\GL\glxext.h                    \
          ..\..\dump\codegen\GL\wgl.h                       \
          ..\..\dump\codegen\GL\wglext.h                    \
          ..\..\dump\codegen\GLES\gl.h                      \
          ..\..\dump\codegen\GLES\glext.h                   \
          ..\..\dump\codegen\GLES2\gl2.h                    \
          ..\..\dump\codegen\GLES2\gl2ext.h                 \
          ..\..\dump\codegen\GLES3\gl3.h 

inputs = codegen.py               \
         input\gl.xml             \
         input\egl.xml            \
         input\glx.xml            \
         input\wgl.xml            \
         wgl-notrace.xml          \
         gl-android.xml           \
         input\reg.py             \
         input\genheaders.py      \
         android-gles1ext.exports \
         android-gles2ext.exports

all: $(outputs)

$(outputs) : $(inputs)
	codegen.py input ..\..\dump\codegen\

clean:
	del ..\..\dump\codegen\*.*
	
	
	
	

