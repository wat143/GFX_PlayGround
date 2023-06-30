#ifndef __CONTEXT_SINGLETON__
#define __CONTEXT_SINGLETON__

#include <iostream>
#include <memory>
#include "Context.h"

class ContextSingleton {
 private:
  static std::shared_ptr<ContextSingleton> singleton_;
  ContextSingleton(Context* c):context(c){}
  Context* context;
 public:
  ContextSingleton()=delete;
  ~ContextSingleton(){}
  void operator=(const ContextSingleton&)=delete;
  static void createInstance(Context* ctx);
  static std::shared_ptr<ContextSingleton> getInstance();
  Context* getContext();
};

#endif

