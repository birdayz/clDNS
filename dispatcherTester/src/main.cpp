#define SPDLOG_TRACE_ON 1

#include <iostream>
#include "UDPReceiver.h"
#include "HashWorker.h"
#include <future>
#include "BatchDispatcher.h"
#include "helper.h"

void test(int platformID, int deviceID, size_t lws, unsigned int iterations, unsigned int domLen) {
    unsigned int batchSize = BATCH_SIZE;
   // unsigned int platformID = 2;
   // unsigned int deviceID = 0;
    unsigned int batchesToDo = 1000;
   // unsigned int iterations = 0;
    unsigned int contexts = 1;
    unsigned int outstanding = 10;


    unsigned int domainSize = domLen;
    unsigned char domain[
            domainSize] = {2, 'x', 'y', 6, 'd', 'n', 's', 'g', 'p', 'u', 5, 'b', 'o', 'g', 'u', 's', 0};
    auto salt = textToSalt("5115CBE8100E470A");

    unsigned char *domains = new unsigned char[batchSize * domainSize];
    unsigned char *salts = new unsigned char[batchSize * salt.size()];


    dataInfoItem *data_info = new dataInfoItem[batchSize];

    for (size_t i = 0; i < batchSize; i++) {
        data_info[i].dataLengthGlobal = domLen;
        data_info[i].saltLengthGlobal = 8;
        data_info[i].iterations = iterations;
        data_info[i].itemDomainLength = 17;
        data_info[i].itemSaltLength = 8;

        memcpy(&domains[i * domainSize], domain, domainSize);
        memcpy(&salts[i * salt.size()], salt.c_str(), salt.size());
    }

   /* unsigned char expected[20] = {218, 94, 172, 88, 217, 73, 196, 96, 162, 34, 69, 208, 197, 111, 7, 38, 52, 220, 149,
                                  60};*/
    unsigned int batchesDone = 0;

    std::mutex mtx;
    contexts =2;
    auto batchDispatcher = std::make_unique<BatchDispatcher>(1,1,1,"/home/j0e/projects/clDNS/gpu/main/kernels",platformID,deviceID,outstanding, contexts, &lws);

    DispatcherTask task;

    task.data_info = data_info;
    task.domain = domains;
    task.salt = salts;
    task.domainLength = domainSize;
    task.saltLength = 8;
    task.batchSize = batchSize;
    task.callback = [&batchesDone, &mtx](
            std::unique_ptr<std::array<std::array<byte, 20>, BATCH_SIZE>> result) {
        std::unique_lock<std::mutex> lck(mtx);
        batchesDone++;
        lck.unlock();
    };

    auto start = Clock::now();
    std::thread t(
            [&batchDispatcher, &task, &batchesToDo]() {
                for (size_t i = 0; i < (batchesToDo/1); i++) {
                    batchDispatcher->dispatch(task);
                }
            });
    t.detach();

    while (true) {
        std::unique_lock<std::mutex> lck(mtx);
        if (batchesDone >= batchesToDo) {
            lck.unlock();
            break;
        }
        lck.unlock();
        std::this_thread::yield();
    }


    duration elapsed = Clock::now() - start;
    std::cout << "=== SUMMARY ===" << std::endl;
    std::cout << "Platform,Device:" << platformID << "," << deviceID << std::endl;
    std::cout << "Iterations:" << iterations << std::endl;
    std::cout << "Batch Size:" << batchSize << std::endl;
    std::cout << "NumBatches:" << batchesToDo << std::endl;
    std::cout << "Total duration (ms): " << elapsed.count() << std::endl;
    long numHashes = (long) batchSize * (long) (iterations + 1) * (long) batchesToDo;
    std::cout << "Items processed: " << numHashes << std::endl;
    std::cout << "Total NSEC3 MH/s:" << ((numHashes) / (elapsed.count() / 1000)) / 1000000 << std::endl;

}

int main(int argc, char *argv[]) {
    int platformId = atoi(argv[1]);
    int deviceId = atoi(argv[2]);
    int lws = atoi(argv[3]);
    test(platformId,deviceId,lws, atoi(argv[4]), atoi(argv[5]));

    return 0;
}