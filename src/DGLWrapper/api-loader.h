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



