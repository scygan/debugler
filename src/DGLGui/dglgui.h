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

#ifndef DGLGUI_H
#define DGLGUI_H

#ifdef DGLGUI_PCH
// Precompiled headers:
#include "dglqtgui.h"
#include "dglcontroller.h"
#include "dglpixelrectangle.h"
#include "dglmainwindow.h"
#include "dgltabbedview.h"
#endif

#include <cassert>

#ifdef NDEBUG
#define CONNASSERT(obj1, slot, obj2, signal) connect(obj1, slot, obj2, signal)
#else
#define CONNASSERT(obj1, slot, obj2, signal) \
    assert(connect(obj1, slot, obj2, signal))

#ifdef HAVE_VLD
#include <vld.h>
#endif

#endif

#endif
