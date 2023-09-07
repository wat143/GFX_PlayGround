#include "ContextSingleton.h"

std::shared_ptr<ContextSingleton> ContextSingleton::singleton_ = nullptr;

void ContextSingleton::createInstance(std::shared_ptr<Context> ctx) {
    if (singleton_ == nullptr)
        singleton_ = std::shared_ptr<ContextSingleton>(new ContextSingleton(ctx));
    else
        std::cerr << "Singleton instance has already been created\n";
}

std::shared_ptr<ContextSingleton> ContextSingleton::getInstance() {
    if (singleton_ == nullptr) {
        std::cerr << "No instance is created\n";
    }
    return singleton_;
}

std::shared_ptr<Context> ContextSingleton::getContext() {
    return context;
}
