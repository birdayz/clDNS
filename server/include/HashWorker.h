#ifndef HASH_WORKER_H
#define HASH_WORKER_H

#define NSEC3_SALT_MAX_LENGTH 8
#define NSEC3_DOMAIN_MAX_LENGTH 40

#include <memory>
#include "UDPSocket.h"
#include "UDPSender.h"
#include "UDPReceiver.h"
#include <thread>
#include <iostream>
#include "NameLookupProcessor.h"
#include <string>
#include <atomic>
#include <pthread.h>
#include <iostream>
#include "OCLExecutor.h"
#include <chrono>
#include "batch.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include "BatchDispatcher.h"
#include "helper.h"

typedef std::chrono::high_resolution_clock Clock;


class HashWorker {
public:
    HashWorker(std::unique_ptr<UDPSender> udpSender, NSEC3Processor &nsec3Processor,
               std::shared_ptr<HashQueue<HashRequest, BATCH_SIZE>> hashQueue,
               std::shared_ptr<BatchDispatcher> batchDispatcher)
            : udpSender(std::move(udpSender)), nsec3Processor(nsec3Processor),
              hashQueue(std::move(hashQueue)), batchDispatcher(batchDispatcher), abort(false) {

    }

    void start() {
        std::thread tr(&HashWorker::doWork, this);
        tr.detach();
    }

    ~HashWorker() {

    }

    static std::unique_ptr<HashWorker> create(NSEC3Processor &nsec3Processor,
                                              std::shared_ptr<HashQueue<HashRequest, BATCH_SIZE>> hashQueue,
                                              std::shared_ptr<UDPSocket> udpSocket,
                                              std::shared_ptr<BatchDispatcher> batchDispatcher) {
        std::unique_ptr<UDPSender> udpSender = std::make_unique<UDPSender>(udpSocket);

        return std::make_unique<HashWorker>(std::move(udpSender), nsec3Processor, std::move(hashQueue),
                                            batchDispatcher);
    }

private:
    void doWork();

    std::unique_ptr<UDPSender> udpSender;
    NSEC3Processor &nsec3Processor;
    std::shared_ptr<HashQueue<HashRequest, BATCH_SIZE>> hashQueue;
    std::shared_ptr<BatchDispatcher> batchDispatcher;
    std::atomic<bool> abort;
};


#endif //HASH_WORKER_H
