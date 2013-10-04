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