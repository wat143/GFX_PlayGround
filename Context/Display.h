#ifndef __DISPLAY__
#define __DISPLAY__

#include "Utils.h"

class Display {
 private:
  int PFType;
 protected:
  unsigned int window_w;
  unsigned int window_h;
 public:
  Display(int type):PFType(type){}
  virtual ~Display(){}
  virtual void* getNativeDisplay()=0;
  virtual void* getNativeWindow()=0;
  int getPFType() { return PFType; };
};

#endif
