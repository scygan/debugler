/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef DEF_H
#define DEF_H

#include <cassert>

#ifdef _WIN32
#define NO_RETURN __declspec(noreturn)
#else
#define NO_RETURN __attribute__((noreturn))
#endif

#ifdef _WIN32
#undef min
#undef max
#endif

// C++11 N2659 "Thread-Local Storage" workarounds
#ifdef _WIN32
#define DGL_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ >= 8))
#define DGL_THREAD_LOCAL thread_local
#else
#define DGL_THREAD_LOCAL __thread
#endif
#else
#define DGL_THREAD_LOCAL thread_local
#endif

#define DGL_ALIGNED(X, A) ((X + A - 1) & (-A))

#ifdef _WIN32
#define strncpy(dest, source, count) strncpy_s(dest, count, source, _TRUNCATE)
#endif

#define DGL_MANUFACTURER  "Slawomir Cygan"
#define DGL_PRODUCT       "Debugler"
#define DGL_PRODUCT_LOWER "debugler"

#define DGL_ASSERT(X) assert(X)

//This macro has a runtime check for accepting arrays only (not pointers).
//http://blogs.msdn.com/b/the1/archive/2004/05/07/128242.aspx
template<typename Type, size_t Size>
char ( &ArraySizeHelper(Type( &Array )[Size]) )[Size];
#define DGL_ARRAY_LENGTH(Array) sizeof(ArraySizeHelper(Array))

#endif
