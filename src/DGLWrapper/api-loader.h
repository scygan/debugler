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


#include <DGLCommon/gl-types.h>

#include <map>

enum ApiLibrary {
    LIBRARY_WGL = 1,
    LIBRARY_EGL = 2,
    LIBRARY_GL  = 4,
    LIBRARY_ES2 = 8,
    LIBRARY_ES3 = 16,

    LIBRARY_GL_EXT = 32,
    LIBRARY_EGL_EXT = 64,
    LIBRARY_WGL_EXT = 128,

    LIBRARY_GLX = 256,
    LIBRARY_NONE = 0
};

class APILoader {
public:
    APILoader();

    void loadLibrary(ApiLibrary apiLibrary);
    void* loadExtPointer(Entrypoint entryp);
    void* ensurePointer(Entrypoint entryp);
private:

    typedef void* LoadedLib;

    void* loadGLPointer(LoadedLib library, Entrypoint entryp);

    std::string getLibraryName(ApiLibrary apiLibrary);

    std::map<std::string, LoadedLib> m_LoadedLibraries;
    int m_LoadedApiLibraries;

    ApiLibrary m_GlueLibrary;
};

extern APILoader g_ApiLoader;



