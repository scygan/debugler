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


#include "dglresourcefs.h"

#include<stdexcept>
#include<cerrno>

#include<QFile>

#undef fopen
#undef fclose
#undef fwrite
#undef fread
#undef ferror
#undef getc
#undef clearerr
#undef fileno



namespace dglfs {

    class DGLResourceFileImpl: public DGLResourceFile, public QFile {
    public:
        DGLResourceFileImpl(const char* name):QFile(name),m_Error(0) {
            if (!open(QIODevice::ReadOnly | QIODevice::Text))
                throw std::runtime_error(errorString().toStdString());
        }

        void setError(int error) { m_Error = error; }
        int getError() { return m_Error; }

        int readDataImpl (char * data, int len) {
            int ret = readData(data, len);
            if (ret < 0) {

            }
            return ret;
        }

    private:
        int m_Error;
    };

    //warning: these commands are not 100% standard conformant, most error scenarios are not implemented (may crash)
    DGLResourceFile* fopen(const char * _Filename, const char * _Mode) {
        try {
            return new DGLResourceFileImpl(_Filename);
        } catch (...) {
            return NULL;
        }
    }

    int fclose(DGLResourceFile * _File) {
        delete _File;
        return 0;
    }

    size_t fwrite(const void *buffer, size_t size, size_t num, DGLResourceFile *stream) {
        reinterpret_cast<DGLResourceFileImpl*>(stream)->setError(ENXIO);
        return 0;
    }

    int ferror(DGLResourceFile * _File) {
        return reinterpret_cast<DGLResourceFileImpl*>(_File)->getError();
    }

    int getc(DGLResourceFile * _File) {
        if (!_File) {
            return EOF;
        }
        DGLResourceFileImpl * file = reinterpret_cast<DGLResourceFileImpl*>(_File);
        char ret;
        if (!file->getChar(&ret)) {
            return EOF;
        } else {
            return ret;
        }
    }

    size_t fread(void * _DstBuf, size_t _ElementSize, size_t _Count, DGLResourceFile * _File) {
        DGLResourceFileImpl * file = reinterpret_cast<DGLResourceFileImpl*>(_File);
        size_t ret = file->readDataImpl((char*)_DstBuf, _ElementSize * _Count);
        if (ret > 0) {
            ret /= _ElementSize;
        } else {
            file->setError(EINVAL);
        }
        return ret;
    }

    void clearerr(DGLResourceFile * _File) {
        return reinterpret_cast<DGLResourceFileImpl*>(_File)->setError(0);
    }

    int fileno(DGLResourceFile * _File) {
        return 0;
    }
};
