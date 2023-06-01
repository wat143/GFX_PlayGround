#ifndef __PICXTFACTORY__
#define __PICXTFACTORY__

#include "ContextFactory.h"

class PiContextFactory : public ContextFactory {
 public:
  PiContextFactory();
  ~PiContextFactory();
  Context* create();
};

#endif
