#ifndef GPUDNS_OCLEXECUTOR_H
#define GPUDNS_OCLEXECUTOR_H

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <CL/cl.h>
#include <array>
#include "batch.h"
#include "helper.h"
#include "Datatypes.h"

typedef unsigned char byte;

class OCLExecutor {
public:
    OCLExecutor(const char *kernelDir, unsigned int platformId, unsigned int deviceId,
                size_t *localWorkgroupSize = nullptr, cl_context ctx = nullptr);

    ~OCLExecutor();

    KernelTask writeToDevice(dataInfoItem *data_info,
                             const unsigned char *domain,
                             const unsigned char *salt,
                             unsigned int domainLength,
                             unsigned int
                             saltLength,
                             int batchSize);

    ReadTask execKernel(cl_mem bufferDataInfo, cl_mem bufferName, cl_mem bufferSalt, cl_mem bufferDigest,
                        size_t batchSize);

    std::unique_ptr<std::array<std::array<byte, 20>, BATCH_SIZE>> readResult(cl_mem bufferDigest, size_t batchSize);

    cl_context getContext() {
        return context;
    }

    static cl_context createContext(unsigned int platformId);

private:
    cl_command_queue commandQueue;
    cl_context context;
    cl_program globalProgram;
    size_t *localWorkgroupSize;

};


#endif //GPUDNS_OCLEXECUTOR_H
