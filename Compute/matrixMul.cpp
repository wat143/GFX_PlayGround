#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/opencl.hpp>

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

void printMatrix(std::vector<float> mat, int w, int h) {
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
    int global_size[2] = {DIMENTION, DIMENTION};
    int local_size[2] = {LOCAL_SIZE, LOCAL_SIZE};
    int width = DIMENTION, height = DIMENTION;
    std::vector<float> vec_a, vec_b, vec_c;
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    /* check global and local size args */
    if (argc >= 3) {
        std::string g_str(static_cast<const char*>(argv[1]));
        std::string l_str(static_cast<const char*>(argv[2]));
        width = height = std::stoi(g_str);
        global_size[0] = global_size[1] = width;
        local_size[0] = local_size[1] = std::stoi(l_str);
        if (argc == 4 && !strcmp(argv[3], "GPUTimeOnly"))
            GPUTimeOnly = true;
    }

    vec_a.resize(width * height);
    vec_b.resize(width * height);
    vec_c.resize(width * height);

    srand(time(0));
    for (int i = 0; i < width * height; i++) {
        vec_a[i] = static_cast<float>(rand() % 10);
        vec_b[i] = static_cast<float>(rand() % 10);
        vec_c[i] = 0;
    }

    if (!GPUTimeOnly)
        std::cout << "=== Mat A ===\n";
    printMatrix(vec_a, width, height);
    if (!GPUTimeOnly)
        std::cout << "=== Mat B ===\n";
    printMatrix(vec_b, width, height);

    /* get OpenCL device and choose 1st one */
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    /* create context */
    cl::Context context({device});
    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

    /* create program */
    cl::Program program(context, mat_mul_kernel);
    program.build({device});

    /* Create clmem for a, b and c */
    /* host_ptr as nullptr since the data will be transffered later */
    /* at enqueue timing. */
    cl::Buffer cl_a(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    sizeof(float) * width * height, vec_a.data());
    cl::Buffer cl_b(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    sizeof(float) * width * height, vec_b.data());
    cl::Buffer cl_c(context, CL_MEM_WRITE_ONLY, sizeof(float) * width * height);

    /* create kernel */
    cl::Kernel kernel(program, "mat_mul_kernel");
    kernel.setArg(0, width);
    kernel.setArg(1, cl_a);
    kernel.setArg(2, cl_b);
    kernel.setArg(3, cl_c);

    /* execute kernel */
    cl::Event event;
    queue.enqueueNDRangeKernel(kernel,
                               cl::NullRange,
                               cl::NDRange(global_size[0], global_size[1]),
                               cl::NDRange(local_size[0], local_size[1]),
                               nullptr, &event);

    queue.finish();
    event.wait();

    /* get result */
    cl::copy(queue, cl_c, vec_c.begin(), vec_c.end());

    cl_ulong start, end;
    start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
    end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

    /* output results */
    if (!GPUTimeOnly)
        std::cout << "=== Mat C ===\n";
    printMatrix(vec_c, width, height);
    std::cout << "GPU time:" << (end - start) * 1.0e-6 << "msec\n";

    return 0;
}
