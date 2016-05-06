#ifndef GPUDNS_NSEC3PROCESSOR_H
#define GPUDNS_NSEC3PROCESSOR_H


#include "helper.h"
#include <vector>
#include "Zone.h"
#include <ldns/ldns.h>
#include "LDNSWrapper.h"
#include "HashQueue.h"
#include <chrono>
#include "batch.h"
#include "ZoneManager.h"

typedef std::chrono::high_resolution_clock Clock;

class NSEC3Processor {
public:
    NSEC3Processor(std::shared_ptr<HashQueue<HashRequest, BATCH_SIZE>> hashQueue) : hashQueue(hashQueue) {

    }

    ~NSEC3Processor() {

    }

    void startProcessing(std::unique_ptr<DNS_REQUEST> request, const Zone &zone);

    std::unique_ptr<DNS_REQUEST> finishProcessing(std::unique_ptr<DNS_REQUEST> request, const Zone &zone,
                                                  NSEC3HashB32 hash);

    std::unique_ptr<DNS_REQUEST> processSync(std::unique_ptr<DNS_REQUEST> request, const Zone &zone);

private:
    std::shared_ptr<HashQueue<HashRequest, BATCH_SIZE>> hashQueue;
};


#endif //GPUDNS_NSEC3PROCESSOR_H
