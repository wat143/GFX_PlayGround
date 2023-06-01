#ifndef __CONTEXTFACTORY__
#define __CONTEXTFACTORY__

#include "Context.h"

class ContextFactory {
 public:
  ContextFactory(){}
  ~ContextFactory(){}
  virtual Context* create()=0;
};

#endif
