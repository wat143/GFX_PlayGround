#ifndef __DRM_DISPLAY__
#define __DRM_DISPLAY__

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <unistd.h>

#include "Display.h"
#include "Utils.h"

/* Note:Legacy commit is the first target and props are not required. */
/*      Prepare for atomic commit expansion */
struct drmObject {
  drmModeObjectProperties* props;
  drmModePropertyRes** propsInfo;
  uint32_t id;
};

struct drmCrtc {
  struct drmObject* crtcObj;
  uint32_t crtcIndex;
};

struct drmFB {
  int fbID;
  struct gbm_bo* bo;
};

struct drmOutput {
  drmModeModeInfo* mode;
  struct drmObject* connector;
  struct drmCrtc* crtc;
  struct drmObject* plane;
  struct drmFB* fb;
  uint32_t blob_id;
};

struct drm {
  int fd;
  struct drmOutput *output;
};

struct gbm {
  struct gbm_device* dev;
  struct gbm_surface* surface;
  struct gbm_bo* bo;
  uint32_t format;
};

class DrmDisplay : public Display {
 private:
  bool firstFlip;
  struct* drm;
  struct* gbm;
  int numBuff = 2; // Use double buffering
  /* DRM related functions */
  int initDrm();
  struct drmOutput* createOutput(drmModeConnector* connector, drmModeRes* res);
  struct drmCrtc* findCrtc(drmModeConnector* connector);
  struct drmObject* findPlane(drmModeConnector* connector);
  bool getObjectProperties(struct drmObject* obj, uint32_t type);
  void drmFBDestroyCallback(struct gbm_bo* bo, void* data);
  struct drmFB* drmFBGetFromBO(struct gbm_bo* bo);
  void pageFlipHandler(int fd, unsigned int frame, unsigned int sec,
		       unsigned int usec, void* data);
  int pageFlip();
  /* GBM related functions */
  struct gbm* initGbm();
  struct gbm* initGbmSurface();
 protected:
  unsigned int window_w;
  unsigned int window_h;
 public:
  DrmDisplay(int type);
  ~DrmDisplay();
  void* getNativeDisplay();
  void* getNativeWindow();
  void* getDisplayDev();
  int pageFlip();
};

#endif
