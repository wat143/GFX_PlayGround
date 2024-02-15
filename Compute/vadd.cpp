#include <CL/cl.h>
#include <vector>
#include <iostream>

#define KERNEL_SIZE 1024
#define LOCAL_SIZE 64

const char *vadd =
    "__kernel void vadd(__global const float *a, \n"
    "                    __global const float *b, \n"
    "                    __global       float *c) \n"
    " {                                           \n"
    "     int gid = get_global_id(0);             \n"
    "     c[gid] = a[gid] + b[gid];               \n"
    " }                                           \n";

int main() {
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
    size_t global_size, local_size;
    std::vector<float> vec_a(KERNEL_SIZE), vec_b(KERNEL_SIZE, 1), vec_c(KERNEL_SIZE, 0);

    // Init vector a
    for (int i = 0; i < KERNEL_SIZE; i++) {
        vec_a[i] = i;
        vec_b[i] = KERNEL_SIZE - i;
    }

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
                          KERNEL_SIZE * sizeof(float),
                          nullptr, &cl_status);
    cl_b = clCreateBuffer(context, CL_MEM_READ_ONLY,
                          KERNEL_SIZE * sizeof(float),
                          nullptr, &cl_status);
    cl_c = clCreateBuffer(context, CL_MEM_READ_ONLY,
                          KERNEL_SIZE * sizeof(float),
                          nullptr, &cl_status);

    // enqueue mem write command
    cl_status = clEnqueueWriteBuffer(command_queue, cl_a, CL_TRUE,
                                     0, KERNEL_SIZE * sizeof(float),
                                     vec_a.data(), 0, nullptr, nullptr);
    cl_status = clEnqueueWriteBuffer(command_queue, cl_b, CL_TRUE,
                                     0, KERNEL_SIZE * sizeof(float),
                                     vec_b.data(), 0, nullptr, nullptr);

    // build source code and create kernel
    program = clCreateProgramWithSource(context, 1,
                                        (const char**)&vadd,
                                        nullptr, &cl_status);
    cl_status = clBuildProgram(program, 1, devices,
                               nullptr, nullptr, nullptr);
    kernel = clCreateKernel(program, "vadd", &cl_status);

    // process kernel
    // add args to kernel
    cl_status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&cl_a);
    cl_status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&cl_b);
    cl_status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&cl_c);

    // execute kernel
    global_size = KERNEL_SIZE;
    local_size = LOCAL_SIZE;
    cl_status = clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr,
                                       &global_size, &local_size, 0,
                                       nullptr, &event);

    // enqueue mem read command
    cl_status = clEnqueueReadBuffer(command_queue, cl_c, CL_TRUE, 0,
                                    KERNEL_SIZE * sizeof(float),
                                    vec_c.data(), 0, nullptr, nullptr);

    // flush command queue and wait task completion
    cl_status = clFlush(command_queue);
    cl_status = clFinish(command_queue);

    // Wait event completion and get start/end time
    clWaitForEvents(1, &event);
    cl_ulong start, end;
    cl_status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                        sizeof(start), &start, nullptr);
    cl_status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                        sizeof(end), &end, nullptr);

    // output results
    for (int i = 0; i < KERNEL_SIZE; i++)
        std::cout << vec_c[i] << " ";
    std::cout << std::endl;
    std::cout << "GPU time:" << (end - start) * 1.0e-6 << "msec\n";

    // clean up
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
