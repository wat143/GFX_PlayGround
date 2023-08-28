#include "DrmContextFactory.h"
#include "EglContext.h"
#include "DrmDisplay.h"
#include "Utils.h"

DrmContextFactory::DrmContextFactory(){}
DrmContextFactory::~DrmContextFactory(){}

Context* DrmContextFactory::create() {
  Display* disp = new DrmDisplay(DispmanX);
  Context* context = new EglContext(disp, OpenGLESv2);
  return context;
}
