#ifndef DGLGUI_H
#define DGLGUI_H

#include<cassert>

#ifdef NDEBUG
#define CONNASSERT(x) x
#else
#define CONNASSERT(x) assert(x)

#ifdef HAVE_VLD
#include <vld.h>
#endif

#endif

#endif
