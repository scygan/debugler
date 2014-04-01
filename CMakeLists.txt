cmake_minimum_required(VERSION 2.8)

project(debugler)

file(STRINGS src/version.inl DGL_VERSION )


add_subdirectory(src)
add_subdirectory(tools/build/cmake)
