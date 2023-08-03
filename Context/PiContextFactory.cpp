#include "PiContextFactory.h"
#include "EglContext.h"
#include "DispmanxDisplay.h"
#include "Utils.h"

PiContextFactory::PiContextFactory(){}
PiContextFactory::~PiContextFactory(){}

Context* PiContextFactory::create() {
  Display* disp = new DispmanxDisplay(DispmanX);
  Context* context = new EglContext(disp, OpenGLESv2);
  return context;
}
