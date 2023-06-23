#include <cassert>
#include <cstdint>
#include "bcm_host.h"
#include "Display.h"
#include "DispmanxDisplay.h"

DispmanxDisplay::DispmanxDisplay(int type):Display(type) {
  int ret;
  bcm_host_init();
  ret = graphics_get_display_size(0, (uint32_t*)&window_w, (uint32_t*)&window_h);
  assert(ret >= 0);
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = window_w;
  dst_rect.height = window_h;
  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = window_w << 16;
  src_rect.height = window_h << 16;        
  dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
  dispman_update = vc_dispmanx_update_start(0);
  dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
					      0/*layer*/, &dst_rect, 0/*src*/,
					      &src_rect, DISPMANX_PROTECTION_NONE,
					      0 /*alpha*/, 0/*clamp*/,
					      DISPMANX_TRANSFORM_T());
  nativewindow.element = dispman_element;
  nativewindow.width = window_w;
  nativewindow.height = window_h;
  vc_dispmanx_update_submit_sync(dispman_update);
};

DispmanxDisplay::~DispmanxDisplay(){
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispmanUpdateHandle;
  dispmanUpdateHandle = vc_dispmanx_update_start(0);
  ret = vc_dispmanx_element_remove(dispmanUpdateHandle, dispman_element);
  assert(ret == 0);
  vc_dispmanx_update_submit_sync(dispmanUpdateHandle);
  ret = vc_dispmanx_display_close(dispman_display);
  assert (ret == 0);
}

void* DispmanxDisplay::getNativeDisplay() {
  return (void*)&dispman_display;
}

void* DispmanxDisplay::getNativeWindow() {
  return (void*)&nativewindow;
}
