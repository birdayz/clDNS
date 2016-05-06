#define SPDLOG_TRACE_ON 1

#include <iostream>
#include "UDPReceiver.h"
#include "testHelper.h"
#include "PacketWorker.h"
#include "HashWorker.h"
#include "BatchDispatcher.h"

int main(int argc, char *argv[]) {
    assert(argc >= 3);

    unsigned int platform = (unsigned int) atoi(argv[1]);
    unsigned int device = (unsigned int) atoi(argv[2]);
    size_t numPacketWorkers = 1;
    size_t numHashWorkers = 1;
    std::shared_ptr<UDPSocket> udpSocket = std::make_shared<UDPSocket>(1337);

    auto hashQueue = std::make_shared<HashQueue<HashRequest, BATCH_SIZE>>();

    std::vector<Zone> zones;
    zones.push_back(constructNew());
    ZoneManager zoneManager(std::move(zones));
    NSEC3Processor nsec3Processor(hashQueue);

    NameLookupProcessor nameLookupProcessor(zoneManager, nsec3Processor);


    std::vector<std::unique_ptr<PacketWorker>> workers;


    for (size_t i = 0; i < numPacketWorkers; i++) {
        std::unique_ptr<PacketWorker> worker = PacketWorker::create(nameLookupProcessor, i, udpSocket);
        worker->start();
        workers.push_back(std::move(worker));
    }

    std::vector<std::unique_ptr<HashWorker>> queueWorkers;


    //size_t localWorkgroupSize = 32;
    auto batchDispatcher = std::make_shared<BatchDispatcher>(1, 1, 1, argv[3], platform, device, 10, 1,
                                                             nullptr);

    for (size_t i = 0; i < numHashWorkers; i++) {
        std::unique_ptr<HashWorker> queueWorker = HashWorker::create(nsec3Processor, hashQueue, udpSocket,
                                                                     batchDispatcher);
        queueWorker->start();
        queueWorkers.push_back(std::move(queueWorker));
    }

    // wait forever, graceful shutdown not implemented yet ;)
    std::mutex mutex;
    bool abort = false;
    std::condition_variable condVar;

    std::unique_lock<std::mutex> lock(mutex);
    while (!abort) {
        condVar.wait(lock);
    }
}