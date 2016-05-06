#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>
#include <string.h>
#include "OCLExecutor.h"

#define CL_CHECK(_expr)                                                         \
   do {                                                                         \
     cl_int _err = _expr;                                                       \
     if (_err == CL_SUCCESS)                                                    \
       break;                                                                   \
     fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err);   \
     abort();                                                                   \
   } while (0)


typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<float, std::milli> duration;

/*
 * Create an OpenCL program from the kernel source file
 */
cl_program createProgram(cl_context context, cl_device_id device, const char *fileName) {
    cl_int errNum;
    cl_program program;

    std::ifstream kernelFile(fileName, std::ios::in);
    if (!kernelFile.is_open()) {
        std::cerr << "Failed to open file for reading: " << fileName << std::endl;
        return NULL;
    }

    std::ostringstream oss;
    oss << kernelFile.rdbuf();

    std::string srcStdStr = oss.str();
    const char *srcStr = srcStdStr.c_str();
    program = clCreateProgramWithSource(context, 1, (const char **) &srcStr, NULL, NULL);
    if (program == NULL) {
        std::cerr << "Failed to create CL program from source." << std::endl;
        return NULL;
    }

    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (errNum != CL_SUCCESS) {
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);

        std::cerr << "Error in kernel: " << std::endl;
        std::cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }

    return program;
}

cl_context OCLExecutor::createContext(unsigned int platformId) {
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId[10];

    // 1. find platform
    errNum = clGetPlatformIDs(10, &firstPlatformId[0], &numPlatforms);
    if (errNum != CL_SUCCESS || numPlatforms <= 0) {
        throw std::logic_error("Failed to find any OpenCL platforms");
    }

    int platformToUse = platformId;
    unsigned int type = CL_DEVICE_TYPE_ALL;
    char buffer[10240];
    cl_context context;
    cl_context_properties contextProperties[] = {CL_CONTEXT_PLATFORM,
                                                 (cl_context_properties) firstPlatformId[platformToUse], 0};
    context = clCreateContextFromType(contextProperties, type, NULL, NULL, &errNum);

    if (errNum != CL_SUCCESS) {
        context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &errNum);
        if (errNum != CL_SUCCESS) {
            throw std::logic_error("Failed to create an OpenCL GPU or CPU context.");
        }
        CL_CHECK(clGetPlatformInfo(firstPlatformId[platformToUse], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL));
        std::cout << "Using CPU" << std::endl;
    }
    else {
        CL_CHECK(clGetPlatformInfo(firstPlatformId[platformToUse], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL));
    }
    return context;
}

OCLExecutor::OCLExecutor(const char *kernelDir, unsigned int platformId, unsigned int deviceId,
                         size_t *localWorkgroupSize, cl_context ctx) : localWorkgroupSize(localWorkgroupSize) {
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId[10];

    // 1. find platform
    errNum = clGetPlatformIDs(10, &firstPlatformId[0], &numPlatforms);
    if (errNum != CL_SUCCESS || numPlatforms <= 0) {
        throw std::logic_error("Failed to find any OpenCL platforms");
    }

    int platformToUse = platformId;
    unsigned int type = CL_DEVICE_TYPE_ALL;
    char buffer[10240];

    // 2. create Context
    if (ctx == nullptr) {
        std::cout << "new ctx" << std::endl;
        cl_context_properties contextProperties[] = {CL_CONTEXT_PLATFORM,
                                                     (cl_context_properties) firstPlatformId[platformToUse], 0};
        context = clCreateContextFromType(contextProperties, type, NULL, NULL, &errNum);

        if (errNum != CL_SUCCESS) {
            context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &errNum);
            if (errNum != CL_SUCCESS) {
                throw std::logic_error("Failed to create an OpenCL GPU or CPU context.");
            }
            CL_CHECK(clGetPlatformInfo(firstPlatformId[platformToUse], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL));
            std::cout << "Using CPU" << std::endl;
        }
        else {
            CL_CHECK(clGetPlatformInfo(firstPlatformId[platformToUse], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL));
        }
    }
    else {
        std::cout << "use other ctx" << std::endl;
        context = ctx;
    }

    std::cout << "Using Platform Name: " << buffer << std::endl;

    if (context == nullptr) {
        throw std::logic_error("Failed to create OpenCL context.");
    }

    // 2. Create commandQ
    size_t deviceBufferSize = -1;
    CL_CHECK(clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize));

    if (deviceBufferSize <= 0) {
        throw std::logic_error("No devices available.");
    }

    cl_device_id devices[deviceBufferSize / sizeof(cl_device_id)];
    CL_CHECK(clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL));
    cl_device_id device = devices[deviceId];

    // Using first available device
    commandQueue = clCreateCommandQueue(context, device, /*CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE*/0, NULL);
    if (commandQueue == nullptr) {
        throw std::logic_error("Failed to create commandQueue for device 0");
    }

    //cl_device_id device = devices[0];
    cl_uint buf_uint;
    cl_ulong buf_ulong;

    // 3. print info
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL));
    CL_CHECK(clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL));

    const char *kernelFile = "/preDataSHA1Batch.cl";

    char path[strlen(kernelDir) + strlen(kernelFile) + 1];
    memcpy(path, kernelDir, strlen(kernelDir));
    memcpy(&path[strlen(kernelDir)], kernelFile, strlen(kernelFile));
    path[strlen(kernelDir) + strlen(kernelFile)] = '\0';

    globalProgram = createProgram(context, device, path);
    if (globalProgram != NULL) { }
}

OCLExecutor::~OCLExecutor() {
    clReleaseProgram(globalProgram);
    clReleaseContext(context);
};

KernelTask OCLExecutor::writeToDevice(dataInfoItem *data_info,
                                      const unsigned char *domain,
                                      const unsigned char *salt,
                                      unsigned int domainLength,
                                      unsigned int
                                      saltLength,
                                      int batchSize) {
    cl_int errNum;

    cl_mem bufferDataInfo = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                           sizeof(dataInfoItem) * batchSize,
                                           NULL, &errNum);
    CL_CHECK(errNum);

    cl_mem bufferName = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                       sizeof(unsigned char) * domainLength * batchSize,
                                       NULL, &errNum);
    CL_CHECK(errNum);

    cl_mem bufferSalt = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                       sizeof(unsigned char) * saltLength * batchSize,
                                       NULL, &errNum);
    CL_CHECK(errNum);

    cl_mem bufferDigest = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(unsigned char) * 20 * batchSize, NULL,
                                         &errNum);
    CL_CHECK(errNum);

    errNum = clEnqueueWriteBuffer(commandQueue, bufferDataInfo, CL_TRUE, 0, sizeof(dataInfoItem) * batchSize,
                                  data_info, 0, nullptr, nullptr);
    CL_CHECK(errNum);

    errNum = clEnqueueWriteBuffer(commandQueue, bufferName, CL_TRUE, 0,
                                  sizeof(unsigned char) * domainLength * batchSize, const_cast<unsigned char *>(domain),
                                  0, nullptr,
                                  nullptr);
    CL_CHECK(errNum);

    errNum = clEnqueueWriteBuffer(commandQueue, bufferSalt, CL_TRUE, 0,
                                  sizeof(unsigned char) * saltLength * batchSize,
                                  const_cast<unsigned char *>(salt), 0, nullptr, nullptr);
    CL_CHECK(errNum);

    KernelTask task;
    task.batchSize = batchSize;
    task.bufferDataInfo = bufferDataInfo;
    task.bufferDomain = bufferName;
    task.bufferSalt = bufferSalt;
    task.bufferDigest = bufferDigest;
    return task;
}

ReadTask OCLExecutor::execKernel(cl_mem bufferDataInfo, cl_mem bufferName, cl_mem bufferSalt, cl_mem bufferDigest,
                                 size_t batchSize) {

    cl_int errNum;
    cl_kernel globalKernel = clCreateKernel(globalProgram, "preparedata_sha1_kernel", NULL);
    if (globalKernel == nullptr) {
        throw std::logic_error("Could not create kernel");
    }

    CL_CHECK(clSetKernelArg(globalKernel, 0, sizeof(cl_mem), &bufferDataInfo));
    CL_CHECK(clSetKernelArg(globalKernel, 1, sizeof(cl_mem), &bufferName));
    CL_CHECK(clSetKernelArg(globalKernel, 2, sizeof(cl_mem), &bufferSalt));
    CL_CHECK(clSetKernelArg(globalKernel, 3, sizeof(cl_mem), &bufferDigest));

    size_t globalWorkSize = batchSize;
    cl_event kernelEvent;

    CL_CHECK(clEnqueueNDRangeKernel(commandQueue, //queue
                                    globalKernel, //kernel
                                    1, //work dimensions
                                    NULL, //NULL, //global offset
                                    &globalWorkSize, //global size TODO ist jetzt nur 1
                                    localWorkgroupSize, //local size
                                    0, //num events in wait list
                                    nullptr, //event wait list
                                    &kernelEvent //event
    ));

    clWaitForEvents(1, &kernelEvent);
    clReleaseEvent(kernelEvent);
    clReleaseKernel(globalKernel);
    clReleaseMemObject(bufferDataInfo);
    clReleaseMemObject(bufferName);
    clReleaseMemObject(bufferSalt);

    ReadTask task;
    task.batchSize = batchSize;
    task.bufferDigest = bufferDigest;

    return task;
}

std::unique_ptr<std::array<std::array<byte, 20>, BATCH_SIZE>> OCLExecutor::readResult(cl_mem bufferDigest,
                                                                                      size_t batchSize) {
    auto hashResult = std::make_unique<std::array<std::array<byte, 20>, BATCH_SIZE>>(); // ALLOC ON HEAP
    CL_CHECK(clEnqueueReadBuffer(commandQueue, bufferDigest, CL_TRUE, 0, sizeof(unsigned char) * 20 * batchSize,
                                 hashResult->data(),
                                 0, nullptr,
                                 nullptr));
    clReleaseMemObject(bufferDigest);
    return std::move(hashResult);
};