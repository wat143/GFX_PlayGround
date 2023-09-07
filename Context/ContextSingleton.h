#ifndef __CONTEXT_SINGLETON__
#define __CONTEXT_SINGLETON__

#include <iostream>
#include <memory>
#include "Context.h"

class ContextSingleton {
private:
    static std::shared_ptr<ContextSingleton> singleton_;
    ContextSingleton(std::shared_ptr<Context> c):context(c){}
    std::shared_ptr<Context> context;
public:
    ContextSingleton()=delete;
    ~ContextSingleton(){}
    void operator=(const ContextSingleton&)=delete;
    static void createInstance(std::shared_ptr<Context> ctx);
    static std::shared_ptr<ContextSingleton> getInstance();
    std::shared_ptr<Context> getContext();
};

#endif
