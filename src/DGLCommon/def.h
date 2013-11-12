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

#ifdef _WIN32
#define NO_RETURN __declspec(noreturn)
#else
#define NO_RETURN __attribute__((noreturn))
#endif

#ifdef _WIN32
#undef min
#endif

// C++11 N2659 "Thread-Local Storage" walkarounds
#ifdef _WIN32
#define THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ >= 8))
#define THREAD_LOCAL thread_local
#else
#define THREAD_LOCAL __thread
#endif
#else
#define THREAD_LOCAL thread_local
#endif

#define ALIGNED(X, A) ((X + A - 1) & (-A))

#ifdef _WIN32
#define strncpy(dest, source, count) strncpy_s(dest, count, source, _TRUNCATE)
#endif

#endif
