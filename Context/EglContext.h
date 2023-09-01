#ifndef __EGLCONTEXT__
#define __EGLCONTEXT__

#include "Context.h"
#include "Display.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

class EglContext : public Context {
 private:
   EGLDisplay display;
   EGLSurface surface;
   EGLContext context;
   NativeDisplay* nativeDisp;
   int fwType;
 public:
  EglContext(NativeDisplay*, int);
   ~EglContext();
   void* getDisplay();
   void* getSurface();
   void* getWindow();
   int makeCurrent();
   int swapBuffers();
};

#endif
