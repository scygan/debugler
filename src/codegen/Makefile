all: nonExtTypedefs.inl wrappers.inl pointers.inl pointers_load.inl functionList.inl OpenGL32.def

nonExtTypedefs.inl wrappers.inl pointers.inl pointers_load.inl functionList.inl OpenGL32.def: input\GL.h input\wgl.h
	codegen.py

clean:
	del ../../dump/codegen/*.*