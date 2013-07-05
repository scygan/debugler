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

#include "native-surface.h"

#ifdef HAVE_LIBRARY_GLX

#include <DGLCommon/os.h>

//for GLXBadDrawable:
#include <X11/Xproto.h>
#include <GL/glxproto.h>
#endif

#include "pointers.h"
#include "api-loader.h"
#include "display.h"

namespace dglState {

NativeSurfaceBase::NativeSurfaceBase(opaque_id_t id):m_Id(id) {}

opaque_id_t NativeSurfaceBase::getId() {
    return m_Id;
}
#ifdef HAVE_LIBRARY_WGL
NativeSurfaceWGL::NativeSurfaceWGL(const DGLDisplayState*, opaque_id_t id):NativeSurfaceBase(id) {
    HDC hdc = reinterpret_cast<HDC>(id);
    int i = DIRECT_CALL_CHK(wglGetPixelFormat)(hdc);
    PIXELFORMATDESCRIPTOR  pfd;
    DIRECT_CALL_CHK(wglDescribePixelFormat)(hdc, i, 
        sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    m_DoubleBuffered = (pfd.dwFlags & PFD_DOUBLEBUFFER) != 0;
    m_Stereo = (pfd.dwFlags & PFD_STEREO) != 0;
    m_RGBASizes[0] = pfd.cRedBits;
    m_RGBASizes[1] = pfd.cGreenBits;
    m_RGBASizes[2] = pfd.cBlueBits;
    m_RGBASizes[3] = pfd.cAlphaBits;
    m_StencilSize = pfd.cStencilBits;
    m_DepthSize = pfd.cDepthBits;
    RECT rc;
    GetClientRect(WindowFromDC(hdc), &rc);
    m_Width = rc.right - rc.left;
    m_Height = rc.bottom - rc.top;
}

bool NativeSurfaceWGL::isDoubleBuffered() {
    return m_DoubleBuffered;
}

bool NativeSurfaceWGL::isStereo() {
    return m_Stereo;
}

int* NativeSurfaceWGL::getRGBASizes() {
    return m_RGBASizes;
}

int NativeSurfaceWGL::getStencilSize() {
    return m_StencilSize;
}

int NativeSurfaceWGL::getDepthSize() {
    return m_DepthSize;
}


int NativeSurfaceWGL::getWidth() {
    return m_Width;
}

int NativeSurfaceWGL::getHeight() {
    return m_Height;
}
#endif
NativeSurfaceEGL::NativeSurfaceEGL(const DGLDisplayState* dpy, opaque_id_t pixfmt, opaque_id_t id):m_Dpy(dpy),NativeSurfaceBase(id) {
    EGLBoolean ret = EGL_TRUE;

    EGLDisplay eglDpy = reinterpret_cast<EGLDisplay>(m_Dpy->getId());
    EGLConfig config = reinterpret_cast<EGLConfig>(pixfmt);

    ret &= DIRECT_CALL_CHK(eglGetConfigAttrib)(eglDpy, config, EGL_RED_SIZE,     &m_RGBASizes[0]);
    ret &= DIRECT_CALL_CHK(eglGetConfigAttrib)(eglDpy, config, EGL_GREEN_SIZE,   &m_RGBASizes[1]);
    ret &= DIRECT_CALL_CHK(eglGetConfigAttrib)(eglDpy, config, EGL_BLUE_SIZE,    &m_RGBASizes[2]);
    ret &= DIRECT_CALL_CHK(eglGetConfigAttrib)(eglDpy, config, EGL_ALPHA_SIZE,   &m_RGBASizes[3]);
    ret &= DIRECT_CALL_CHK(eglGetConfigAttrib)(eglDpy, config, EGL_DEPTH_SIZE,   &m_DepthSize);
    ret &= DIRECT_CALL_CHK(eglGetConfigAttrib)(eglDpy, config, EGL_STENCIL_SIZE, &m_StencilSize);
    if (!ret) {
        throw std::runtime_error("eglGetConfigAttrib failed during native surface pixelformat discovery");
    }
}

bool NativeSurfaceEGL::isDoubleBuffered() {
    return true;
}

bool NativeSurfaceEGL::isStereo() {
    return false;
}

int* NativeSurfaceEGL::getRGBASizes() {
    return m_RGBASizes;
}

int NativeSurfaceEGL::getStencilSize() {
    return m_StencilSize;
}

int NativeSurfaceEGL::getDepthSize() {
    return m_DepthSize;
}

int NativeSurfaceEGL::getWidth() {
    EGLint width;
    EGLBoolean ret;

    EGLSurface surface = reinterpret_cast<EGLSurface>(m_Id);
    EGLDisplay dpy = reinterpret_cast<EGLDisplay>(m_Dpy->getId());

    ret = DIRECT_CALL_CHK(eglQuerySurface)(dpy, surface, EGL_WIDTH, &width);
    if (!ret) {
        throw std::runtime_error("eglQuerySurface failed during native surface pixelformat discovery");
    }
    return width;
}

int NativeSurfaceEGL::getHeight() {
    EGLint height;
    EGLBoolean ret;
    
    EGLSurface surface = reinterpret_cast<EGLSurface>(m_Id);
    EGLDisplay dpy = reinterpret_cast<EGLDisplay>(m_Dpy->getId());

    ret = DIRECT_CALL_CHK(eglQuerySurface)(dpy, surface, EGL_HEIGHT, &height);
    if (!ret) {
        throw std::runtime_error("eglQuerySurface failed during native surface pixelformat discovery");
    }
    return height;
}

#ifdef HAVE_LIBRARY_GLX

class XErrorHandler {
public:
    XErrorHandler(Display* display):m_Lock(s_mtx) {
        s_disp = display;
        s_oldErrorHandler = XSetErrorHandler(s_errorHandler);
    }
    ~XErrorHandler() {
        XSetErrorHandler(s_oldErrorHandler);
    }
    int getErrorCode() {
        XSync(s_disp, False);
        int errorCode = 0;
        std::swap(errorCode, s_errorCode);
        return errorCode;
    }
private:
    static int s_errorHandler(Display *display, XErrorEvent *error) {
        if (s_disp != display) {
            if (s_oldErrorHandler) {
                return s_oldErrorHandler(display, error);
            } else {
                assert(0);
            }
            return 0; 
        }
        if (error->error_code) {
           int errorBase, eventBase;
           glXQueryExtension(display, &errorBase, &eventBase);
           s_errorCode = error->error_code - errorBase;
       }
    }

    static Display* s_disp;
    static std::mutex s_mtx;
    static int (*s_oldErrorHandler)(Display *, XErrorEvent *);
    static int s_errorCode;

    //whole class should be used once at a time, so we lock thorugh it lifetime.
    std::lock_guard<std::mutex> m_Lock;
};


std::mutex XErrorHandler::s_mtx;
int XErrorHandler::s_errorCode = 0;
Display* XErrorHandler::s_disp;
int (*XErrorHandler::s_oldErrorHandler)(Display *, XErrorEvent *);


NativeSurfaceGLX::NativeSurfaceGLX(const DGLDisplayState* _dpy, opaque_id_t id):m_Dpy(_dpy),NativeSurfaceBase(id), m_GLXDrawableGettersFailing(false) {
    
    GLXDrawable drawable = static_cast<GLXDrawable>(m_Id);
    Display* dpy = reinterpret_cast<Display*>(m_Dpy->getId());

    unsigned int fbConfigID = 0xbadf00d;
    int error; 
    {
        XErrorHandler xErrorHandler(dpy);
        DIRECT_CALL_CHK(glXQueryDrawable)(dpy, drawable, GLX_FBCONFIG_ID, &fbConfigID);
        error = xErrorHandler.getErrorCode();
    }
    GLXFBConfig* config = NULL, * memToFree = NULL;
    if (!error) {
        const int attribList[] = {GLX_FBCONFIG_ID, static_cast<int>(fbConfigID), None, None};
        int nElements;
        config = DIRECT_CALL_CHK(glXChooseFBConfig)(dpy, DefaultScreen(dpy), attribList, &nElements);
        if (!nElements) {
            config = NULL;
        }
        memToFree = config;
    }

    if (!config && (error == GLXBadDrawable || !error)) {
        //this is acceptable and happen on Mesa, if bare Window XID was passed here instead of GLXDrawable
        //this condition is signalized with GLXBadDrawable, but on INDIRECT rendering there is mysteriously no error (?)
        
        m_GLXDrawableGettersFailing = true;

        Window win = reinterpret_cast<Window>(id);

        //in such case try to talk directly with X
        
        XWindowAttributes attribs;
        XGetWindowAttributes(dpy, win, &attribs);

        const int visualID = XVisualIDFromVisual(attribs.visual);

        int nElements; 
        GLXFBConfig* configs = DIRECT_CALL_CHK(glXGetFBConfigs)(dpy, DefaultScreen(dpy), &nElements);

        config = NULL;
        if (nElements) {
            memToFree = configs;
        }

        for (int i =0; i < nElements; i++) {
            int id;
            if (DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, configs[i], GLX_VISUAL_ID, &id) == Success) {
                if (id == visualID) {
                    config = &configs[i];
                    break;
                }
            }
        }
    }

    if (!config) {
        Os::fatal("Cannot get FBConfig for given GLXDrawable/Window XID");
    }
    

    int ret = Success;
    
    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_RED_SIZE,   &m_RGBASizes[0]);
    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_GREEN_SIZE, &m_RGBASizes[1]);
    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_BLUE_SIZE,  &m_RGBASizes[2]);
    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_ALPHA_SIZE, &m_RGBASizes[3]);

    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_DEPTH_SIZE,   &m_DepthSize);
    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_STENCIL_SIZE, &m_StencilSize);

    int val;
    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_DOUBLEBUFFER, &val); m_DoubleBuffered = val;

    ret |= DIRECT_CALL_CHK(glXGetFBConfigAttrib)(dpy, *config, GLX_STEREO, &val); m_Stereo = val;
 
    if (ret != Success) {
       throw std::runtime_error("eglGetConfigAttrib failed during native surface pixelformat discovery");
    }

    XFree(memToFree);
}

bool NativeSurfaceGLX::isDoubleBuffered() {
    return m_DoubleBuffered;
}

bool NativeSurfaceGLX::isStereo() {
    return m_Stereo;
}

int* NativeSurfaceGLX::getRGBASizes() {
    return m_RGBASizes;
}

int NativeSurfaceGLX::getStencilSize() {
    return m_StencilSize;
}

int NativeSurfaceGLX::getDepthSize() {
    return m_DepthSize;
}

int NativeSurfaceGLX::getWidth() {

    Display* dpy = reinterpret_cast<Display*>(m_Dpy->getId());

    if (!m_GLXDrawableGettersFailing) {

        GLXDrawable drawable = static_cast<GLXDrawable>(m_Id);

        unsigned int width;
        DIRECT_CALL_CHK(glXQueryDrawable)(dpy, drawable, GLX_WIDTH, &width);

        return width;

    } else {
        Window win = reinterpret_cast<Window>(m_Id);

        XWindowAttributes attribs;
        XGetWindowAttributes(dpy, win, &attribs);
        return attribs.width;
    }
}

int NativeSurfaceGLX::getHeight() {
    Display* dpy = reinterpret_cast<Display*>(m_Dpy->getId());

    if (!m_GLXDrawableGettersFailing) {

        GLXDrawable drawable = static_cast<GLXDrawable>(m_Id);

        unsigned int height;
        DIRECT_CALL_CHK(glXQueryDrawable)(dpy, drawable, GLX_HEIGHT, &height);

        return height;

    } else {
        Window win = reinterpret_cast<Window>(m_Id);

        XWindowAttributes attribs;
        XGetWindowAttributes(dpy, win, &attribs);
        return attribs.height;
    }
}

#endif

} //namespace dglState
