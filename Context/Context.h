#ifndef __CONTEXT__
#define __CONTEXT__

#include "Display.h"

class Context {
 protected:
  Display* Disp;
  int API;
 public:
  Context(Display* disp, int api):Disp(disp),API(api){}
  virtual ~Context(){ delete Disp; }
  virtual void* getWindow()=0;
  virtual void* getDisplay()=0;
  virtual void* getSurface()=0;
  virtual int makeCurrent()=0;
  virtual int swapBuffers()=0;
};

#endif
