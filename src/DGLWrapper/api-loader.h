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

#ifndef API_LOADER_H
#define API_LOADER_H

#include <DGLCommon/gl-types.h>

#include <map>

enum ApiLibrary {
    LIBRARY_WGL = 1,
    LIBRARY_EGL = 2,
    LIBRARY_GL = 4,
    LIBRARY_ES1 = 8,
    LIBRARY_ES1_EXT = 16,
    LIBRARY_ES2 = 32,
    LIBRARY_ES2_EXT = 64,
    LIBRARY_ES3 = 128,
    LIBRARY_GL_EXT = 256,
    LIBRARY_EGL_EXT = 512,
    LIBRARY_WGL_EXT = 1024,
    LIBRARY_GLX = 2048,
    LIBRARY_GLX_EXT = 4096,
    LIBRARY_NONE = 0
};

/**
 * Class utilising various APIs to load all OpenGL entrypoints
 */
class APILoader {
   public:
    APILoader();

    /**
     * Load whole API library
     *
     * Called on initialization (for loading WGL/GLX/EGL
     * Called on context creation (for loading specific api, like GL or ES2)
     */
    void loadLibrary(ApiLibrary apiLibrary);

    /**
     * Load one extension pointer
     *
     * This is called when app calls *glGetProcAddressMethod, to ensure
     * proper extension pointer is loaded. Should not be used for
     * non-ext APIs
     *
     * @ret: true on success load, false if implementation does not support this
     *entrypoint
     */
    bool loadExtPointer(Entrypoint entryp);

    typedef void* LoadedLib;

    /**
     * Ensure pointer is loaded
     *
     * This function is used mainly for debugger calls. It just checks
     * the pointer and loads it if not loaded earlier.
     *
     * Throws in pointer is not supported by implementation.
     *
     * Usable with all entrypoints, but all non-EXT entrypoints
     * should be loaded earlier with loadLibrary()
     */
    FUNC_PTR ensurePointer(Entrypoint entryp);

    /**
     * Small util deciding if given library name is interesting to debugger
     *
     * Used on *nix platform, when we can intercept loader calls. This checks
     * if filename in dlopen() is interresting for us
     */
    bool isLibGL(const char* name);

    /**
     * Set pointer to entrypoint implementation
     *
     * Used internally.
     * On *nix platform used also externally: if we can intercept loader calls,
     * this is called on dlsym call from application. This saves loader time, as
     * it does not need to call dlsym itself again.
     */
    void setPointer(Entrypoint entryp, FUNC_PTR impl);

   private:
    std::string getLibraryName(ApiLibrary apiLibrary);
    FUNC_PTR loadGLPointer(LoadedLib library, Entrypoint entryp);

    std::map<std::string, LoadedLib> m_LoadedLibraries;
    int m_LoadedApiLibraries;

    ApiLibrary m_GlueLibrary;
};

extern APILoader g_ApiLoader;

#endif    // API_LOADER_H
