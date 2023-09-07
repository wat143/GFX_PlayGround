#ifndef __CONTEXTFACTORY__
#define __CONTEXTFACTORY__

#include "Context.h"

class ContextFactory {
public:
    ContextFactory(){}
    ~ContextFactory(){}
    Context* create(int fw_type);
};

#endif
