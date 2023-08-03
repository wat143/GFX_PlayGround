#ifndef __DISPLAY__
#define __DISPLAY__

#include "Utils.h"

class Display {
 private:
  int FWType;
 protected:
  unsigned int window_w;
  unsigned int window_h;
 public:
  Display(int type):FWType(type){}
  virtual ~Display(){}
  virtual void* getNativeDisplay()=0;
  virtual void* getNativeWindow()=0;
  int getFWType() { return FWType; };
};

#endif
