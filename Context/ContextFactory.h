#ifndef __CONTEXTFACTORY__
32;331;0c#define __CONTEXTFACTORY__

#include "Context.h"

class ContextFactory {
 public:
  ContextFactory(){}
  ~ContextFactory(){}
  Context* create(int fw_type);
};

#endif
