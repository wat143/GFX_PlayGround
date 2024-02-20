#include <vector>
#include <iostream>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/opencl.hpp>

#define KERNEL_SIZE 1024
#define LOCAL_SIZE 64

const std::string vadd =
    "__kernel void vadd(__global const float *a, \n"
    "                    __global const float *b, \n"
    "                    __global       float *c) \n"
    " {                                           \n"
    "     int gid = get_global_id(0);             \n"
    "     c[gid] = a[gid] + b[gid];               \n"
    " }                                           \n";

int main() {
    /* get platform and use 1st one */
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    /* get OpenCL device and choose 1st one */
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    /* create context */
    cl::Context context({device});
    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

    /* create program */
    cl::Program program(context, vadd);
    program.build({device});
  
    std::vector<float> vec_a(KERNEL_SIZE), vec_b(KERNEL_SIZE, 1), vec_c(KERNEL_SIZE, 0);
    /*Init vector */
    for (int i = 0; i < KERNEL_SIZE; i++) {
        vec_a[i] = i;
        vec_b[i] = KERNEL_SIZE - i;
    }

    /* Create clmem for a, b and c */
    /* host_ptr as nullptr since the data will be transffered later */
    /* at enqueue timing. */
    cl::Buffer cl_a(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * KERNEL_SIZE, vec_a.data());
    cl::Buffer cl_b(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * KERNEL_SIZE, vec_b.data());
    cl::Buffer cl_c(context, CL_MEM_WRITE_ONLY, sizeof(float) * KERNEL_SIZE);

    /* create kernel */
    cl::Kernel kernel(program, "vadd");
    kernel.setArg(0, cl_a);
    kernel.setArg(1, cl_b);
    kernel.setArg(2, cl_c);

    /* execute kernel */
    cl::Event event;
    queue.enqueueNDRangeKernel(kernel,
                               cl::NullRange,
                               cl::NDRange(KERNEL_SIZE),
                               cl::NDRange(LOCAL_SIZE),
                               nullptr, &event);
    queue.finish();

    /* get result */
    cl::copy(queue, cl_c, vec_c.begin(), vec_c.end());

    // Wait event completion and get start/end time
    event.wait();
    cl_ulong start, end;
    start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
    end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

    // output results
    for (int i = 0; i < KERNEL_SIZE; i++)
        std::cout << vec_c[i] << " ";
    std::cout << std::endl;
    std::cout << "GPU time:" << (end - start) * 1.0e-6 << "msec\n";

    return 0;
}
