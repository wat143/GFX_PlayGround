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
    drm->useAtomic = supportAtomic();
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
    bool isFound = false;

    crtc->crtcObj = new struct drmObject;
    for (int i = 0; !isFound && i < connector->count_encoders; i++) {
        encoder = drmModeGetEncoder(drm->fd, connector->encoders[i]);
        if (!encoder)
            continue;

        for (int j = 0; !isFound && j < res->count_crtcs; j++) {
            /* check if CRTC can work with this encoder */
            if (!(encoder->possible_crtcs & (1 << j)))
                continue;
            crtcId = res->crtcs[j];
            if (crtcId > 0) {
                crtc->crtcObj->id = crtcId;
                crtc->crtcIndex = j;
                isFound = true;
            }
        }
        drmModeFreeEncoder(encoder);
    }
    if (!isFound) {
        std::cerr << "cannot find CRTC for connector "
                  << connector->connector_id << std::endl;
        delete crtc->crtcObj;
        delete crtc;
        crtc = nullptr;
    }
    return crtc;
}

struct drmObject* DrmDisplay::findPlane(uint32_t crtcIdx) {
    drmModePlaneResPtr planeRes;
    bool isFound = false;
    struct drmObject* plane = new struct drmObject;

    planeRes = drmModeGetPlaneResources(drm->fd);
    if (!planeRes) {
        std::cerr << "drmModeGetPlaneResources failed " << strerror(errno) << std::endl;
        delete plane;
        return nullptr;
    }

    for (int i = 0; (i < planeRes->count_planes) && !isFound; i++) {
        int planeId = planeRes->planes[i];
        drmModePlanePtr currentPlane = drmModeGetPlane(drm->fd, planeId);
        if (!currentPlane) {
            std::cerr << "drmModeGetPlane(" << planeId << ") failed "
                      << strerror(errno) << std::endl;
            continue;
        }
        if (currentPlane->possible_crtcs & (1 << crtcIdx)) {
            plane->id = planeId;
            isFound = true;
        }
    }
    drmModeFreePlaneResources(planeRes);
    if (!isFound) {
        std::cerr << "cannot find primary plane\n";
        delete plane;
        return nullptr;
    }
    return plane;
}

static int setDrmObjectProp(drmModeAtomicReq* req, struct drmObject* obj,
                                   std::string name, uint64_t val) {
    uint32_t prop_id = 0;
    for (int i = 0; i < obj->props->count_props; i++) {
        if (!strcmp(obj->propsInfo[i]->name, name.c_str())) {
            prop_id = obj->propsInfo[i]->prop_id;
            break;
        }
    }
    if (prop_id == 0) {
        std::cerr << "No object property " << name << std::endl;
        return -EINVAL;
    }
    return drmModeAtomicAddProperty(req, obj->id, prop_id, val);
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
    int area = 0;
    /* Choose mode */
    for (int i = 0; i < connector->count_modes; i++) {
        drmModeModeInfo *currentMode = &connector->modes[i];
        /* Use prefered mode if exists */
        if (currentMode->type & DRM_MODE_TYPE_PREFERRED) {
            output->mode = currentMode;
            window_w = currentMode->hdisplay;
            window_h = currentMode->vdisplay;
            break;
        }
        /* use largest mode */
        int currentArea = currentMode->hdisplay * currentMode->vdisplay;
        if (area < currentArea) {
            output->mode = currentMode;
            window_w = currentMode->hdisplay;
            window_h = currentMode->vdisplay;
            area = currentArea;
        }
    }
    std::cout << "Mode (w, h) = "<< window_w << ", " << window_h << std::endl;  
    struct drmObject* drmConnector = new struct drmObject;
    drmConnector->id = connector->connector_id;
    output->connector = drmConnector;
    /* create the blob id required for atomic commit */
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
    output->plane = findPlane(output->crtc->crtcIndex);

    /* get object properties of connector, CRTC and plane */
    if (!getObjectProperties(output->connector, DRM_MODE_OBJECT_CONNECTOR))
        std::cerr << "Failed to get connector object properties\n";
    if (!getObjectProperties(output->crtc->crtcObj, DRM_MODE_OBJECT_CRTC))
        std::cerr << "Failed to get CRTC object properties\n";
    if (!getObjectProperties(output->plane, DRM_MODE_OBJECT_PLANE))
        std::cerr << "Failed to get plane object properties\n";
    std::cout << "=== DRM initialization completed\n";
    std::cout << "=== Connector: " << output->connector->id
              << " CRTC: " << output->crtc->crtcObj->id 
              << " Plane: " << output->plane->id << std::endl;
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
                /* 1 output is enough as of now */
                break;

            }
            else
                std::cerr << "Failed to create output from connector "
                          << connector->connector_id;
        }
        drmModeFreeConnector(connector);
    }
    drmModeFreeResources(resources);
    return 0;
}

static void drmFBDestroyCallback(struct gbm_bo* bo, void* data) {
    struct drmFB* fb = static_cast<struct drmFB*>(data);
    int fd = gbm_device_get_fd(gbm_bo_get_device(bo));
    if (fb->id)
        drmModeRmFB(fd, fb->id);
    delete fb;
}

struct drmFB* DrmDisplay::drmFBGetFromBO(struct gbm_bo* bo) {
    struct drmFB *fb = static_cast<struct drmFB*>(gbm_bo_get_user_data(bo));
    uint32_t width, height, format;
    uint32_t strides[4] = {0}, handles[4] = {0}, offsets[4] = {0}, flags = 0;
    int ret = -1;

    if (fb)
        return fb;

    fb = static_cast<struct drmFB*>(calloc(1, sizeof(struct drmFB*)));
    fb->bo = bo;
    width = gbm_bo_get_width(bo);
    height = gbm_bo_get_height(bo);
    format = gbm_bo_get_format(bo);
    /* get bo data */
    if (gbm_bo_get_handle_for_plane && gbm_bo_get_modifier &&
        gbm_bo_get_plane_count && gbm_bo_get_stride_for_plane &&
        gbm_bo_get_offset) {
        uint64_t modifiers[4] = {0};
        modifiers[0] = gbm_bo_get_modifier(bo);
        const int num_planes = gbm_bo_get_plane_count(bo);
        for (int i = 0; i < num_planes; i++) {
            handles[i] = gbm_bo_get_handle_for_plane(bo, i).u32;
            strides[i] = gbm_bo_get_stride_for_plane(bo, i);
            offsets[i] = gbm_bo_get_offset(bo, i);
            modifiers[i] = modifiers[0];
        }

        if (modifiers[0] && modifiers[0] != DRM_FORMAT_MOD_INVALID) {
            flags = DRM_MODE_FB_MODIFIERS;
            std::cout << "Using modifier " << modifiers[0] << std::endl;
        }
        ret = drmModeAddFB2WithModifiers(drm->fd, width, height,
                                         format, handles, strides, offsets,
                                         modifiers, &fb->id, flags);
    }


    if (ret) {
        if (flags)
            std::cerr <<  "FB creation with modifiers failed\n";

        memcpy(handles, 0, 16);
        handles[0] = gbm_bo_get_handle(bo).u32;
        memcpy(strides, 0, 16);
        strides[0] = gbm_bo_get_stride(bo);
        memset(offsets, 0, 16);
        ret = drmModeAddFB2(drm->fd, width, height, format,
                            handles, strides, offsets, &fb->id, 0);
    }

    if (ret) {
        std::cerr << "Failed to create FB: " << strerror(errno) << std::endl;
        delete fb;
        return nullptr;
    }

    gbm_bo_set_user_data(bo, fb, drmFBDestroyCallback);
    std::cout << "FB created " << fb->id << std::endl;
    return fb;
}

bool DrmDisplay::supportAtomic() {
    int ret = 0;
    bool support;
    struct drmOutput* output = drm->output;
    drmModeAtomicReq* req = drmModeAtomicAlloc();
    if (setDrmObjectProp(req, output->connector, "CRTC_ID", output->crtc->crtcObj->id) < 0)
        ret = -1;
    if (!ret && setDrmObjectProp(req, output->crtc->crtcObj, "MODE_ID", output->blob_id) < 0)
        ret = -1;
    if (!ret)
        ret = drmModeAtomicCommit(drm->fd, req, DRM_MODE_ATOMIC_TEST_ONLY, nullptr);
    if (ret < 0)
        support = false;
    else
        support = true;
    drmModeAtomicFree(req);
    return support;
}

static void pageFlipHandler(int fd, unsigned int frame, unsigned int sec,
                            unsigned int usec, void* data)
{
    int* pendingFlip = static_cast<int*>(data);
    *pendingFlip = 0;
}

int DrmDisplay::pageFlipLegacy() {
    fd_set fds;
    struct gbm_bo* bo;
    struct drmOutput* output = drm->output;
    uint32_t crtcId, connId;
    int ret, pendingFlip = 1;
    drmEventContext eventCtx = {
        .version = 2,
        .page_flip_handler = pageFlipHandler,
    };
    crtcId = output->crtc->crtcObj->id;
    connId = output->connector->id;
    if (firstFlip) {
        firstFlip = false;
        gbm->bo = gbm_surface_lock_front_buffer(gbm->surface);
        output->fb = drmFBGetFromBO(gbm->bo);
        if (!output->fb) {
            std::cerr << "failed to get FB " << strerror(errno) << std::endl;
            return ret;
        }
        std::cout << "Connector ID: " << connId << ", "
                  << "CRTC ID:" << crtcId << ", "
                  << "FB ID: " << output->fb->id << std::endl;
        ret = drmModeSetCrtc(drm->fd, crtcId, output->fb->id,
                             0, 0, &connId, 1, output->mode);
        if (ret) {
            std::cerr << "failed to set mode " << errno << std::endl;
            return ret;
        }
    }
    else {
        struct gbm_bo* next_bo;
        next_bo = gbm_surface_lock_front_buffer(gbm->surface);
        output->fb = drmFBGetFromBO(next_bo);
        if (!output->fb) {
            std::cerr << "failed to get FB " << strerror(errno) << std::endl;
            return ret;
        }
        pendingFlip = 1;
        ret = drmModePageFlip(drm->fd, crtcId, output->fb->id,
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

static void pageFlipHandlerAtomic(int fd, unsigned int frame, unsigned int sec,
                                  unsigned int usec, unsigned int crtc_id, void* data)
{
    int* pendingFlip = static_cast<int*>(data);
    *pendingFlip = 0;
}

static int atomicCommit(struct drm* drm, int width, int height, int flags) {
    int ret;
    drmModeAtomicReq* req;
    struct drmObject* connector = drm->output->connector;
    struct drmObject* crtc = drm->output->crtc->crtcObj;
    struct drmObject* plane = drm->output->plane;
    struct drmFB* fb = drm->output->fb;

    req = drmModeAtomicAlloc();
    if (flags & DRM_MODE_ATOMIC_ALLOW_MODESET) {
        /* set CRTC ID to use */
        if (setDrmObjectProp(req, connector, "CRTC_ID", crtc->id) < 0)
            return -1;
        /* set mode id for CRTC */
        if (setDrmObjectProp(req, crtc, "MODE_ID", drm->output->blob_id) < 0)
            return -1;
        /* set CRTC active */
        if (setDrmObjectProp(req, crtc, "ACTIVE", 1) < 0)
            return -1;
    }

    /* set plane properties */
    if (setDrmObjectProp(req, plane, "FB_ID", fb->id) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "CRTC_ID", crtc->id) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "SRC_X", 0) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "SRC_Y", 0) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "SRC_W", width << 16) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "SRC_H", height << 16) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "CRTC_X", 0) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "CRTC_Y", 0) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "CRTC_W", width) < 0)
        return -1;
    if (setDrmObjectProp(req, plane, "CRTC_H", height) < 0)
        return -1;

	flags |= DRM_MODE_PAGE_FLIP_EVENT;
    ret = drmModeAtomicCommit(drm->fd, req, flags, nullptr);
    if (ret < 0)
        std::cerr << "atomic commit failed\n";

    drmModeAtomicFree(req);

    return ret;
}

int DrmDisplay::pageFlipAtomic() {
    int flags = 0;
    fd_set fds;
    struct gbm_bo* next_bo;
    struct drmOutput* output = drm->output;
    uint32_t crtcId, connId;
    int ret, pendingFlip = 1;
    drmEventContext eventCtx = {
        .version = 3,
        .page_flip_handler2 = pageFlipHandlerAtomic,
    };
    crtcId = output->crtc->crtcObj->id;
    connId = output->connector->id;

    next_bo = gbm_surface_lock_front_buffer(gbm->surface);
    output->fb = drmFBGetFromBO(next_bo);
    if (!output->fb) {
        std::cerr << "failed to get FB " << strerror(errno) << std::endl;
        return ret;
    }
    pendingFlip = 1;
    if (firstFlip) {
        flags = DRM_MODE_ATOMIC_ALLOW_MODESET;
        firstFlip = false;
    }
    atomicCommit(drm, window_w, window_h, flags);
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
        } else if (FD_ISSET(drm->fd, &fds)) {
            drmHandleEvent(drm->fd, &eventCtx);
        }
    }
    gbm_surface_release_buffer(gbm->surface, gbm->bo);
    gbm->bo = next_bo;

    return 0;
}

int DrmDisplay::pageFlip() {
    if (drm->useAtomic)
        return pageFlipAtomic();
    else
        return pageFlipLegacy();
}

int DrmDisplay::initGbmSurface(uint64_t modifier) {
    /* ToDo: use gbm_surface_create_with_modifiers in future. */
	if (gbm_surface_create_with_modifiers) {
		gbm->surface = 
            gbm_surface_create_with_modifiers(gbm->dev, window_w,
                                              window_h, gbm->format,
                                              &modifier, 1);
	}

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
