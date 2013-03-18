#include <DGLCommon/gl-types.h>

#include <map>

enum ApiLibrary {
    LIBRARY_WGL = 1,
    LIBRARY_EGL = 2,
    LIBRARY_GL  = 4,
    LIBRARY_ES2 = 8,

    LIBRARY_GL_EXT = 0,
    LIBRARY_EGL_EXT = 0,
    LIBRARY_WGL_EXT = 0,
    LIBRARY_NONE = 0
};

class APILoader {
public:
    APILoader::APILoader();

    void loadLibrary(ApiLibrary apiLibrary);
    void* loadExtPointer(Entrypoint entryp);
    void* ensurePointer(Entrypoint entryp);
private:

    typedef void* LoadedLib;

    void* loadGLPointer(LoadedLib library, Entrypoint entryp);

    std::string getLibraryName(ApiLibrary apiLibrary);

    std::map<std::string, LoadedLib> m_LoadedLibraries;

    ApiLibrary m_GlueLibrary;
};

extern APILoader g_ApiLoader;



