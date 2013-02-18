#include <cstdio>
#include <fstream>

#define FILE DGLResourceFile

class DGLResourceFile {
public:
    virtual ~DGLResourceFile() {}
};


namespace dglfs {
    DGLResourceFile* fopen(const char * _Filename, const char * _Mode);
    int fclose(DGLResourceFile * _File);
    size_t fwrite(const void *buffer, size_t size, size_t num, DGLResourceFile *stream);
    int ferror(DGLResourceFile * _File);
    int getc(DGLResourceFile * _File);
    size_t fread(void * _DstBuf, size_t _ElementSize, size_t _Count, DGLResourceFile * _File);
    void clearerr(DGLResourceFile * _File);
    int fileno(DGLResourceFile * _File);
};

#define fopen dglfs::fopen
#define fclose dglfs::fclose
#define fwrite dglfs::fwrite
#define fread dglfs::fread
#define ferror dglfs::ferror
#define getc dglfs::getc
#define clearerr dglfs::clearerr
#define fileno dglfs::fileno


#undef stdout
#undef stdin
//currently unsupported
#define stdout NULL
#define stdin NULL