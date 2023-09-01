#include <errno.h>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <cassert>
#include "DrmDisplay.h"

DrmDisplay::DrmDisplay(int type):NativeDisplay(type){
  drm = new struct drm;
  if(initDrm("/dev/dri/card1"))
    std::cerr << "Failed to init DRM\n";
  /* ToDo: format and modifier shall be configurable */
  gbm = new struct gbm;
  if (initGbm(DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR))
    std::cerr << "Failed to init GBM\n";
  firstFlip = true;
}

DrmDisplay::~DrmDisplay() {
}

void* DrmDisplay::getNativeDisplay() {
  /* NOP */
  return nullptr;
}

void* DrmDisplay::getNativeWindow() {
  /* provide gbm surface */
  return gbm->surface;
}

void* DrmDisplay::getDisplayDev() {
  return gbm->dev;
}

struct drmCrtc* DrmDisplay::findCrtc(drmModeRes* res, drmModeConnector* connector) {
  drmModeEncoder* encoder = nullptr;
  struct drmCrtc* crtc = new struct drmCrtc;
  uint32_t crtcId = 0;

  crtc->crtcObj = new struct drmObject;
  if (connector->encoder_id)
    encoder = drmModeGetEncoder(drm->fd, connector->connector_id);
  else
    encoder = nullptr;

  if (encoder) {
    if (encoder->crtc_id)
      crtcId = encoder->crtc_id;
    if (crtcId > 0) {
      drmModeFreeEncoder(encoder);
      crtc->crtcObj->id = crtcId;
      for (int i = 0; i < res->count_crtcs; i++) {
	if (res->crtcs[i] == crtcId) {
	  crtc->crtcIndex = i;
	  break;
	}
      }
      return crtc;
    }
    drmModeFreeEncoder(encoder);
  }

  /* Following 2 cases need to find available encoder+CRTC pair from all available resources */
  /*     1. Connector is not bound to an encoder.                                            */
  /*     2. CRTC and encoder pair has already been used.                                     */
  for (int i = 0; i < connector->count_encoders; i++) {
    encoder = drmModeGetEncoder(drm->fd, connector->encoders[i]);
    if (!encoder)
      continue;

    for (int j = 0; j < res->count_crtcs; j++) {
      /* check if CRTC can work with this encoder */
      if (!(encoder->possible_crtcs & (1 << j)))
	continue;
      crtcId = res->crtcs[j];
      if (crtcId > 0) {
	drmModeFreeEncoder(encoder);
	crtc->crtcObj->id = crtcId;
	crtc->crtcIndex = j;
	return crtc;
      }
    }
    drmModeFreeEncoder(encoder);
  }
  std::cerr << "cannot find CRTC for connector " << connector->connector_id << std::endl;
  delete crtc->crtcObj;
  delete crtc;
  return nullptr;
}

struct drmObject* DrmDisplay::findPlane(uint32_t crtcId) {
  drmModePlaneResPtr planeRes;
  bool isPrimary = false;
  struct drmObject* plane = new struct drmObject;

  planeRes = drmModeGetPlaneResources(drm->fd);
  if (!planeRes) {
    std::cerr << "drmModeGetPlaneResources failed " << strerror(errno) << std::endl;
    delete plane;
    return nullptr;
  }

  for (int i = 0; (i < planeRes->count_planes) && !isPrimary; i++) {
    int planeId = planeRes->planes[i];
    drmModePlanePtr currentPlane = drmModeGetPlane(drm->fd, planeId);
    if (!currentPlane) {
      std::cerr << "drmModeGetPlane(" << planeId << ") failed " << strerror(errno) << std::endl;
      continue;
    }
    if (currentPlane->possible_crtcs & (1 << crtcId)) {
      /* check primary plane or not */
      drmModeObjectPropertiesPtr props =
	drmModeObjectGetProperties(drm->fd, planeId, DRM_MODE_OBJECT_PLANE);
      for (int j = 0; j < props->count_props && !isPrimary; j++) {
	drmModePropertyPtr prop = drmModeGetProperty(drm->fd, props->props[j]);
	if (!strcmp(prop->name, "type") && props->prop_values[j] == DRM_PLANE_TYPE_PRIMARY) {
	  isPrimary = true;
	  plane->id = planeId;
	}
	drmModeFreeProperty(prop);
      }
      drmModeFreeObjectProperties(props);
    }
  }
  drmModeFreePlaneResources(planeRes);
  if (!isPrimary) {
    std::cerr << "cannot find primary plane\n";
    delete plane;
    return nullptr;
  }
  return plane;
}

bool DrmDisplay::getObjectProperties(struct drmObject* obj, uint32_t type) {
  obj->props = drmModeObjectGetProperties(drm->fd, obj->id, type);
  if (!obj->props)
    return false;
  obj->propsInfo = new drmModePropertyRes*[obj->props->count_props];
  for (int i = 0; i < obj->props->count_props; i++)
    obj->propsInfo[i] = drmModeGetProperty(drm->fd, obj->props->props[i]);
  return true;
}

struct drmOutput* DrmDisplay::createOutput(drmModeConnector* connector, drmModeRes* res) {
  struct drmOutput* output = new struct drmOutput;
  /* Choose mode */
  /* Use prefered mode */
  for (int i = 0; i < connector->count_modes; i++) {
    drmModeModeInfo *currentMode = &connector->modes[i];
    if (currentMode->type & DRM_MODE_TYPE_PREFERRED) {
      output->mode = currentMode;
      break;
    }
  }
  struct drmObject* drmConnector = new struct drmObject;
  drmConnector->id = connector->connector_id;
  output->connector = drmConnector;
  /* create the blob id */
  if (drmModeCreatePropertyBlob(drm->fd, output->mode,
				sizeof(*output->mode), &output->blob_id) != 0) {
    std::cerr << "Failed to create a blob\n";
    delete output->connector;
    delete output;
    return nullptr;
  }
  /* find CRTC */
  output->crtc = findCrtc(res, connector);
  /* find Plane */
  /* We always use primary plane for rendering */
  output->plane = findPlane(output->crtc->crtcObj->id);

  /* get object properties of connector, CRTC and plane */
  if (!getObjectProperties(output->connector, DRM_MODE_OBJECT_CONNECTOR))
    std::cerr << "Failed to get connector object properties\n";
  if (!getObjectProperties(output->crtc->crtcObj, DRM_MODE_OBJECT_CRTC))
    std::cerr << "Failed to get CRTC object properties\n";
  if (!getObjectProperties(output->plane, DRM_MODE_OBJECT_PLANE))
    std::cerr << "Failed to get plane object properties\n";

  return output;
}

int DrmDisplay::initDrm(std::string dev_name) {
  drmModeRes *resources = nullptr;
  drmModeConnector *connector = nullptr;
  int ret;
  
  assert(drm);
  drm->fd = open(dev_name.c_str(), O_RDWR);
  resources = drmModeGetResources(drm->fd);
  if (resources == nullptr) {
    std::cerr << "Failed to get DRM resources " << errno << std::endl;
    return -1 * errno;
  }
  for (int i = 0; i < resources->count_connectors; i++) {
    connector = drmModeGetConnector(drm->fd, resources->connectors[i]);
    if (connector && connector->connection == DRM_MODE_CONNECTED) {
      drm->output = createOutput(connector, resources);
      if (drm->output) {
	drmModeFreeConnector(connector);
	break;
      }
      else
	std::cerr << "Failed to create output from connector " << connector->connector_id;
    }
    drmModeFreeConnector(connector);
  }
  drmModeFreeResources(resources);
  return 0;
}

static void drmFBDestroyCallback(struct gbm_bo* bo, void* data) {
  struct drmFB* fb = static_cast<struct drmFB*>(data);
  int fd = gbm_device_get_fd(gbm_bo_get_device(bo));
  if (fb->fbID)
    drmModeRmFB(fd, fb->fbID);
  delete fb;
}

struct drmFB* DrmDisplay::drmFBGetFromBO(struct gbm_bo* bo) {
  struct drmFB *fb = static_cast<struct drmFB*>(gbm_bo_get_user_data(bo));
  uint32_t width, height, format;
  uint32_t strides[4] = {0}, handles[4] = {0}, offsets[4] = {0};
  int ret;

  if (fb)
    return fb;

  fb = static_cast<struct drmFB*>(calloc(1, sizeof(struct drmFB*)));
  fb->bo = bo;
  /* get bo data */
  width = gbm_bo_get_width(bo);
  height = gbm_bo_get_height(bo);
  format = gbm_bo_get_format(bo);
  handles[0] = gbm_bo_get_handle(bo).u32;
  strides[0] = gbm_bo_get_stride(bo);
  ret = drmModeAddFB2(drm->fd, width, height, format,
		      handles, strides, offsets,
                      &fb->fbID, 0);
  if (ret) {
    std::cerr << "Failed to create FB: " << strerror(errno) << std::endl;
    delete fb;
    return nullptr;
  }
  gbm_bo_set_user_data(bo, fb, drmFBDestroyCallback);
  return fb;
}

static void pageFlipHandler(int fd, unsigned int frame, unsigned int sec,
				 unsigned int usec, void* data)
{
  int* pendingFlip = static_cast<int*>(data);
  *pendingFlip = 0;
}

int DrmDisplay::pageFlip() {
  fd_set fds;
  struct gbm_bo* bo;
  uint32_t crtcId;
  int ret, pendingFlip = 1;
  drmEventContext eventCtx = {
    .version = 2,
    .page_flip_handler = pageFlipHandler,
  };
  crtcId = drm->output->crtc->crtcObj->id;
  if (firstFlip) {
    struct drmOutput* output = drm->output;
    gbm->bo = gbm_surface_lock_front_buffer(gbm->surface);
    output->fb = drmFBGetFromBO(gbm->bo);
    ret = drmModeSetCrtc(drm->fd, crtcId, output->fb->fbID,
			 0, 0, &output->connector->id, 1, output->mode);
    if (ret) {
      std::cerr << "failed to set mode " << strerror(errno) << std::endl;
      return ret;
    }
    firstFlip = false;
  }
  else {
    struct drmOutput* output = drm->output;
    struct gbm_bo* next_bo;
    next_bo = gbm_surface_lock_front_buffer(gbm->surface);
    output->fb = drmFBGetFromBO(next_bo);
    if (!output->fb) {
      std::cerr << "failed to set mode " << strerror(errno) << std::endl;
      return ret;
    }
    pendingFlip = 1;
    ret = drmModePageFlip(drm->fd, crtcId, output->fb->fbID,
                          DRM_MODE_PAGE_FLIP_EVENT, &pendingFlip);
    while (pendingFlip) {
      FD_ZERO(&fds);
      FD_SET(0, &fds);
      FD_SET(drm->fd, &fds);
      ret = select(drm->fd + 1, &fds, nullptr, nullptr, nullptr);
      if (ret < 0) {
	std::cerr << "select err: " << strerror(errno) << std::endl;
	return ret;
      } else if (ret == 0) {
	std::cerr << "select timeout\n";
	return -1;
      } else if (FD_ISSET(0, &fds)) {
	std::cerr << "User interrupt\n";
	return -1;
      }
      drmHandleEvent(drm->fd, &eventCtx);
    }
    gbm_surface_release_buffer(gbm->surface, gbm->bo);
    gbm->bo = next_bo;
  }
  return 0;
}

int DrmDisplay::initGbmSurface(uint64_t modifier) {
  /* ToDo: use gbm_surface_create_with_modifiers in future. */
  if (!gbm->surface) {
    gbm->surface = gbm_surface_create(gbm->dev, window_w, window_h, gbm->format,
				      GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
  }
  if (!gbm->surface) {
    std::cerr << "Failed to create GBM surface\n";
    return -1;
  }
  return 0;
}

int DrmDisplay::initGbm(uint32_t format, uint64_t modifier) {
  gbm->dev = gbm_create_device(drm->fd);
  if (!gbm->dev) {
    std::cerr << "Failed to create GBM device\n";
    return -1;
  }
  gbm->format = format;
  gbm->surface = nullptr;
  return initGbmSurface(modifier);
}
