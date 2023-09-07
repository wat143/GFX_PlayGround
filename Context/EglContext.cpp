#include <iostream>
#include <cassert>
#include "EglContext.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "Utils.h"

EglContext::EglContext(NativeDisplay* disp, int api):Context(disp, api){
  int ret = -1;
  EGLint num_config = 0;
  const EGLint attribute_list[] =
   {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 24,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE
   };

  const EGLint context_attributes[] = 
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

  nativeDisp = disp;
  fwType = disp->getFWType();
   // Get eglDisplay
  void* dev = disp->getDisplayDev();
  if (!dev)
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  else
    display = eglGetDisplay(static_cast<EGLNativeDisplayType>(dev));
  assert(display != EGL_NO_DISPLAY);

   // Initialize the EGL display connection
  ret = eglInitialize(display, nullptr, nullptr);
  assert(ret != EGL_FALSE);

  // Get an appropriate EGL frame buffer configuration
  EGLConfig config;
  ret = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
  assert(ret != EGL_FALSE);

  // Bind GLESv2
  if (API == OpenGLESv2)
      ret = eglBindAPI(EGL_OPENGL_ES_API);
  else
      ret = EGL_FALSE;
  assert(ret != EGL_FALSE);
   
  // Create an EGL rendering context
  if (fwType == DRM)
      context =
          eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
  else
      context =
          eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
  assert(context != EGL_NO_CONTEXT);

  // Create an EGL window surface
  surface = eglCreateWindowSurface(display,
                                   config,
                                   reinterpret_cast<EGLNativeWindowType>(disp->getNativeWindow()),
                                   nullptr);
  assert(surface != EGL_NO_SURFACE);
  eglMakeCurrent(display, surface, surface, context);
}

EglContext::~EglContext(){
  // Clear Display state
  if (fwType != DRM)
    eglSwapBuffers(display, surface);

   // Release resources
   eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(display, context);
   eglTerminate(display );
}

void* EglContext::getDisplay() { return (void*)&display; }

void* EglContext::getSurface() { return (void*)&surface; }

void* EglContext::getWindow() { return nullptr; }

int EglContext::makeCurrent() {
  int ret;
  ret = eglMakeCurrent(display, surface, surface, context);
  return ret;
}

int EglContext::swapBuffers() {
  int ret;
  if (fwType == DRM) {
    ret = eglSwapBuffers(display, surface);
    nativeDisp->pageFlip();
  }
  else
    ret = eglSwapBuffers(display, surface);
  return ret;
}
