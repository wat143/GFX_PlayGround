#ifndef __DRMCXTFACTORY__
#define __DRMCXTFACTORY__

#include "ContextFactory.h"

class DrmContextFactory : public ContextFactory {
 public:
  DrmContextFactory();
  ~DrmContextFactory();
  Context* create();
};

#endif
