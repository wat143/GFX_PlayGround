#include <CL/cl.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define DIMENTION 1024
#define LOCAL_SIZE 32

bool GPUTimeOnly = false;

const char *mat_mul_kernel =
    "__kernel void mat_mul_kernel(int width, \n"
    "                    __global float *a, \n"
    "                    __global float *b, \n"
    "                    __global float *c) { \n"
    "    int col = get_global_id(0); \n"
    "    int row = get_global_id(1); \n"
    "    float result = 0.0; \n"
    "    for (int i = 0; i < width; i++) { \n"
    "        result += a[row * width + i] * b[i * width + col]; \n"
    "    } \n"
    "    c[row * width + col] = result; \n"
    "} \n";

void printMatrix(float *mat, int w, int h) {
    if (GPUTimeOnly)
        return;
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            std::cout << mat[i * w + j] << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char **argv) {
    cl_platform_id *platforms = nullptr;
    cl_device_id *devices = nullptr;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem cl_a , cl_b, cl_c;
    cl_program program;
    cl_kernel kernel;
    cl_uint num_platforms;
    cl_uint num_devices;
    cl_int cl_status;
    cl_event event;
    size_t global_size[2] = {DIMENTION, DIMENTION}, local_size[2] = {LOCAL_SIZE, LOCAL_SIZE};
    float *a, *b, *c;
    int width = DIMENTION, height = DIMENTION;

    /* check global and local size args */
    if (argc >= 3) {
        std::string g_str(static_cast<const char*>(argv[1])), l_str(static_cast<const char*>(argv[2]));
        width = height = std::stoi(g_str);
        global_size[0] = global_size[1] = width;
        local_size[0] = local_size[1] = std::stoi(l_str);
        if (argc == 4 && !strcmp(argv[3], "GPUTimeOnly"))
            GPUTimeOnly = true;
    }

    a = new float[width * height];
    b = new float[width * height];
    c = new float[width * height];

    srand(time(0));
    for (int i = 0; i < width * height; i++) {
        a[i] = static_cast<float>(rand() % 10);
        b[i] = static_cast<float>(rand() % 10);
        c[i] = 0;
    }

    if (!GPUTimeOnly)
        std::cout << "=== Mat A ===\n";
    printMatrix(a, width, height);
    if (!GPUTimeOnly)
        std::cout << "=== Mat B ===\n";
    printMatrix(b, width, height);

    // Query platform num
    cl_status = clGetPlatformIDs(0, nullptr, &num_platforms);
    // Get platform
    platforms = new cl_platform_id[num_platforms];
    cl_status = clGetPlatformIDs(num_platforms, platforms, nullptr);
    // Query device num
    cl_status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU,
                               0, nullptr, &num_devices);
    // Create device
    devices = new cl_device_id[num_devices];
    cl_status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU,
                               num_devices, devices, nullptr);
    // Create context
    context = clCreateContext(nullptr, num_devices, devices,
                              nullptr, nullptr, &cl_status);
    // Create command queue
    command_queue =
        clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE, &cl_status);

    // Create clmem for a, b and c
    // host_ptr as nullptr since the data will be transffered later
    // at enqueue timing.
    cl_a = clCreateBuffer(context, CL_MEM_READ_ONLY,
                          width * height * sizeof(float),
                          nullptr, &cl_status);
    cl_b = clCreateBuffer(context, CL_MEM_READ_ONLY,
                          width * height * sizeof(float),
                          nullptr, &cl_status);
    cl_c = clCreateBuffer(context, CL_MEM_READ_ONLY,
                          width * height * sizeof(float),
                          nullptr, &cl_status);

    // enqueue mem write command
    cl_status = clEnqueueWriteBuffer(command_queue, cl_a, CL_TRUE,
                                     0, width * height * sizeof(float),
                                     a, 0, nullptr, nullptr);
    cl_status = clEnqueueWriteBuffer(command_queue, cl_b, CL_TRUE,
                                     0, width * height * sizeof(float),
                                     b, 0, nullptr, nullptr);
    // build source code and create kernel
    program = clCreateProgramWithSource(context, 1,
                                        (const char**)&mat_mul_kernel,
                                        nullptr, &cl_status);
    cl_status = clBuildProgram(program, 1, devices,
                               nullptr, nullptr, nullptr);
    kernel = clCreateKernel(program, "mat_mul_kernel", &cl_status);

    // process kernel
    // add args to kernel
    cl_status = clSetKernelArg(kernel, 0, sizeof(int), (void *)&width);
    cl_status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&cl_a);
    cl_status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&cl_b);
    cl_status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&cl_c);

    // execute kernel
    cl_status = clEnqueueNDRangeKernel(command_queue, kernel, 2, nullptr,
                                       &global_size[0], &local_size[0], 0,
                                       nullptr, &event);
    // enqueue mem read command
    cl_status = clEnqueueReadBuffer(command_queue, cl_c, CL_TRUE, 0,
                                    width * height * sizeof(float),
                                    c, 0, nullptr, nullptr);

    // flush command queue and wait task completion
    cl_status = clFlush(command_queue);
    cl_status = clFinish(command_queue);

    // GPU time measurement
    cl_ulong start = 0, end = 0;
    cl_status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                        sizeof(start), &start, nullptr);
    cl_status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                        sizeof(end), &end, nullptr);

    // output results
    if (!GPUTimeOnly)
        std::cout << "=== Mat C ===\n";
    printMatrix(c, width, height);
    std::cout << "GPU time:" << (end - start) * 1.0e-6 << "msec\n";

    // Free resources
    cl_status = clReleaseKernel(kernel);
    cl_status = clReleaseProgram(program);
    cl_status = clReleaseMemObject(cl_a);
    cl_status = clReleaseMemObject(cl_b);
    cl_status = clReleaseMemObject(cl_c);
    cl_status = clReleaseCommandQueue(command_queue);
    cl_status = clReleaseContext(context);
    delete[] devices;
    delete[] platforms;

    return 0;
}
