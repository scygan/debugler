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


#ifndef NATIVE_SURFACE_H
#define NATIVE_SURFACE_H

#include <DGLCommon/gl-types.h>

class DGLDisplayState;

namespace dglState {

//Native platform interface surface.
class NativeSurfaceBase {
public:
    NativeSurfaceBase(opaque_id_t);
    opaque_id_t getId();
    virtual bool isDoubleBuffered() = 0;
    virtual bool isStereo() = 0;

    virtual int* getRGBASizes() = 0;
    virtual int getStencilSize() = 0;
    virtual int getDepthSize() = 0;

    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual ~NativeSurfaceBase() {}
protected:
    opaque_id_t m_Id;
};

class NativeSurfaceWGL: public NativeSurfaceBase {
public:
    NativeSurfaceWGL(const DGLDisplayState* dpy, opaque_id_t id);
    virtual bool isDoubleBuffered();
    virtual bool isStereo();

    virtual int* getRGBASizes();
    virtual int getStencilSize();
    virtual int getDepthSize();

    virtual int getWidth(); 
    virtual int getHeight(); 
private:
    int m_Width, m_Height;
    bool m_Stereo, m_DoubleBuffered;
    int m_RGBASizes[4];
    int m_DepthSize, m_StencilSize;
};

class NativeSurfaceGLX: public NativeSurfaceBase {
public:
    /**
     * Ctor
     */
    NativeSurfaceGLX(const DGLDisplayState* dpy, opaque_id_t id);

    virtual bool isDoubleBuffered();
    virtual bool isStereo();

    virtual int* getRGBASizes();
    virtual int getStencilSize();
    virtual int getDepthSize();

    virtual int getWidth(); 
    virtual int getHeight();

#ifdef HAVE_LIBRARY_GLX
    static GLXFBConfig* getFbConfigForVisual(Display *dpy, VisualID visualID, GLXFBConfig** memToFree);
#endif

private:
    int m_RGBASizes[4];
    int m_DepthSize, m_StencilSize;
    const DGLDisplayState* m_Dpy;
    bool m_Stereo, m_DoubleBuffered;

    bool m_GLXDrawableGettersFailing;
};

class NativeSurfaceEGL: public NativeSurfaceBase {
public:
    /**
     * Ctor
     */
    NativeSurfaceEGL(const DGLDisplayState* dpy, opaque_id_t pixfmt, opaque_id_t id);

    virtual bool isDoubleBuffered();
    virtual bool isStereo();

    virtual int* getRGBASizes();
    virtual int getStencilSize();
    virtual int getDepthSize();

    virtual int getWidth(); 
    virtual int getHeight();
private:
    int m_RGBASizes[4];
    int m_DepthSize, m_StencilSize;
    const DGLDisplayState* m_Dpy;
};

} //namespace
#endif
