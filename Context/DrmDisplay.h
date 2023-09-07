#ifndef __DRM_DISPLAY__
#define __DRM_DISPLAY__

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <gbm.h>
#include <unistd.h>

#include "Display.h"
#include "Utils.h"

/****************************************************************/
/* Refered kmscube: https://gitlab.freedesktop.org/mesa/kmscube */
/****************************************************************/

/*** GBM ***/
struct gbm {
  struct gbm_device* dev;
  struct gbm_surface* surface;
  struct gbm_bo* bo;
  uint32_t format;
};

/*** DRM ***/
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
  uint32_t id;
  struct gbm_bo* bo;
};

struct drmOutput {
  drmModeModeInfo* mode;
  struct drmObject* connector;
  struct drmCrtc* crtc;
  struct drmObject* plane;
  struct drmFB* fb;
  uint32_t blob_id; // used for mode setup in atomic commit
};

struct drm {
  int fd;
  struct drmOutput *output;
};

class DrmDisplay : public NativeDisplay {
 private:
  bool firstFlip;
  struct gbm* gbm;
  struct drm* drm;
  int numBuff = 2; // Use double buffering
  /* DRM related functions */
  int initDrm(std::string devName);
  struct drmOutput* createOutput(drmModeConnector* connector, drmModeRes* res);
  struct drmCrtc* findCrtc(drmModeRes* res, drmModeConnector* connector);
  struct drmObject* findPlane(uint32_t crtcId);
  bool getObjectProperties(struct drmObject* obj, uint32_t type);
  struct drmFB* drmFBGetFromBO(struct gbm_bo* bo);
  /* GBM related functions */
  int initGbm(uint32_t format, uint64_t modifier);
  int initGbmSurface(uint64_t modifier);
 public:
  DrmDisplay(int type);
  ~DrmDisplay();
  void* getNativeDisplay();
  void* getNativeWindow();
  void* getDisplayDev();
  int pageFlip();
};

#endif
