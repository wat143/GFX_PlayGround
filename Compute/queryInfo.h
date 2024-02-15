#ifndef __QUERY_INFO__
#define __QUERY_INFO__

#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <map>

int querySize = 1024;

const std::vector<int> platformQueries = {
    CL_PLATFORM_NAME,
    CL_PLATFORM_VENDOR,
    CL_PLATFORM_VERSION,
    CL_PLATFORM_PROFILE,
    CL_PLATFORM_EXTENSIONS
};

const std::map<int, std::string> platformQueriesStr = {
    {CL_PLATFORM_NAME, "CL_PLATFORM_NAME"},
    {CL_PLATFORM_VENDOR, "CL_PLATFORM_VENDOR"},
    {CL_PLATFORM_VERSION, "CL_PLATFORM_VERSION"},
    {CL_PLATFORM_PROFILE, "CL_PLATFORM_PROFILE"},
    {CL_PLATFORM_EXTENSIONS, "CL_PLATFORM_EXTENSIONS"}
};

const std::map<int, std::string> deviceQueries = {
    {CL_DEVICE_NAME, "STR"},
    {CL_DEVICE_VENDOR, "STR"},
    {CL_DEVICE_VERSION, "STR"},
    {CL_DRIVER_VERSION, "STR"},
    {CL_DEVICE_VERSION, "STR"},
    {CL_DEVICE_MAX_COMPUTE_UNITS, "INT"}
};

const std::map<int, std::string> deviceQueriesStr = {
    {CL_DEVICE_NAME, "CL_DEVICE_NAME"},
    {CL_DEVICE_VENDOR, "CL_DEVICE_VENDOR"},
    {CL_DEVICE_VERSION, "CL_DEVICE_VERSION"},
    {CL_DRIVER_VERSION, "CL_DRIVER_VERSION"},
    {CL_DEVICE_VERSION, "CL_DEVICE_VERSION"},
    {CL_DEVICE_MAX_COMPUTE_UNITS, "CL_DEVICE_MAX_COMPUTE_UNITS"}
};

void printPlatformInfo(cl_platform_id platform) {
    char q_str[querySize] = {'\0'};
    for (int query:platformQueries) {
        clGetPlatformInfo(platform, query, querySize, q_str, nullptr);
        std::cout << platformQueriesStr.at(query) << ": " << q_str << std::endl;
    }
}

void printDeviceInfo(cl_device_id device) {
    char q_str[querySize] = {'\0'};
    int q_int = -1;
    for (auto query:deviceQueries) {
        if (query.second == "STR") {
            clGetDeviceInfo(device, query.first, querySize, q_str, nullptr);
            std::cout << deviceQueriesStr.at(query.first) << ": " << q_str << std::endl;
        }
        else if (query.second == "INT") {
            clGetDeviceInfo(device, query.first, sizeof(int), &q_int, nullptr);
            std::cout << deviceQueriesStr.at(query.first) << ": " << q_int << std::endl;
        }
    }
}

#endif
