#include <CL/cl.h>
#include "queryInfo.h"

int main() {
    cl_platform_id *platforms;
    cl_device_id *devices;
    cl_uint num_platforms = 0, num_devices = 0;
    cl_int status;

    /* Get platform */
    status = clGetPlatformIDs(0, nullptr, &num_platforms);
    platforms = new cl_platform_id[num_platforms];
    status = clGetPlatformIDs(num_platforms, platforms, nullptr);
    printPlatformInfo(platforms[0]);
    /* Get device */
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU,
                            0, nullptr, &num_devices);
    devices = new cl_device_id[num_devices];
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU,
                            num_devices, devices, nullptr);
    printDeviceInfo(devices[0]);

    /* release resources */
    delete[] devices;
    delete[] platforms;

    return 0;

}
