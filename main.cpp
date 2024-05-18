// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <CL/opencl.hpp>
#include <vector>
#include <string>
#include <cmath>


void check_opencl_error(cl_int err, std::string msg) {
    if (err != CL_SUCCESS) {
        std::cerr << "Error: " << msg << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}



int main(int argc, char* argv[]) {
    cl_int err;

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    check_opencl_error(platforms.empty() ? -1 : CL_SUCCESS, "No platforms found");

    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    check_opencl_error(devices.empty() ? -1 : CL_SUCCESS, "No devices found");

    cl::Device device = devices.front();

    cl::Context context(device);

    cl::CommandQueue queue(context, device, 0, &err);
    check_opencl_error(err, "Failed to created command queue");

    // OpenCL kernel
    const char* kernelSource = R"(
        __kernel void integral(__global const float* range, __global float* result, const float step, const int num_steps) {
            int gid = get_global_id(0);
            float x = range[0] + gid * step;
            result[gid] = x * x * step;     
        }
    )";

    float a = 0.0f;
    float b = 1.0f;
    int num_steps = 10000;
    float step = (b - a) / num_steps;

    std::vector<float> results(num_steps, 0);

    cl::Buffer bufferRange(
        context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * 2,
        (void*)&a
    );

    cl::Buffer bufferResult(
        context,
        CL_MEM_WRITE_ONLY,
        sizeof(float) * num_steps
    );

    cl::Program program(context, kernelSource, true, &err);
    check_opencl_error(err, "Failed to build program");

    cl::Kernel kernel(program, "integral", &err);
    check_opencl_error(err, "Failed to create kernel");

    kernel.setArg(0, bufferRange);
    kernel.setArg(1, bufferResult);
    kernel.setArg(2, step);
    kernel.setArg(3, num_steps);

    cl::NDRange global(num_steps);
    cl::Event event;
    err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange, nullptr, &event);
    check_opencl_error(err, "Failed to enqueue kernel");

    event.wait();

    err = queue.enqueueReadBuffer(bufferResult, CL_TRUE, 0, sizeof(float) * num_steps, results.data());
    check_opencl_error(err, "Failed to read buffer");

    float integral_result = 0.0f;
    for (int i = 0; i < num_steps; ++i) {
        integral_result += results[i];
    }

    std::cout << "Integral of x^2 from " << a << " to " << b << " is " << integral_result << std::endl;

    return 0;
}
