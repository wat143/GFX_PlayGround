#ifndef __DISPLAY__
#define __DISPLAY__

#include "Utils.h"

class NativeDisplay {
 private:
  int FWType;
 protected:
  unsigned int window_w;
  unsigned int window_h;
 public:
  NativeDisplay(int type):FWType(type){}
  virtual ~NativeDisplay(){}
  virtual void* getNativeDisplay()=0;
  virtual void* getNativeWindow()=0;
  virtual void* getDisplayDev()=0;
  virtual int pageFlip()=0;
  int getFWType() { return FWType; };
};

#endif
