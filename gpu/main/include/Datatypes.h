#ifndef GPUDNS_DATATYPES_H
#define GPUDNS_DATATYPES_H

#include <memory>
#include <array>
#include "batch.h"

typedef std::function<void(
        std::unique_ptr<std::array<std::array<byte, 20>, BATCH_SIZE>>)> DispatcherCallback;

typedef struct DispatcherTask {
    bool empty = false;
    dataInfoItem *data_info;
    const unsigned char *domain;
    const unsigned char *salt;
    unsigned int domainLength;
    unsigned int saltLength;
    int batchSize;
    DispatcherCallback callback;
} DispatcherTask;

typedef struct KernelTask {
    bool empty = false;
    cl_mem bufferDataInfo;
    cl_mem bufferDomain;
    cl_mem bufferSalt;
    cl_mem bufferDigest;
    size_t batchSize;
    DispatcherCallback callback;
} KernelTask;

typedef struct ReadTask {
    bool empty = false;
    cl_mem bufferDigest;
    size_t batchSize;
    DispatcherCallback callback;
} ReadTask;


#endif //GPUDNS_DATATYPES_H
