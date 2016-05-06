#ifndef GPUDNS_BATCHDISPATCHER_H
#define GPUDNS_BATCHDISPATCHER_H

#include <thread>
#include "helper.h"
#include "batch.h"
#include "blockingconcurrentqueue.h"
#include "InputQueueWorker.h"
#include "KernelQueueWorker.h"
#include "ReadQueueWorker.h"
#include "Datatypes.h"


class BatchDispatcher {
public:
    BatchDispatcher(size_t numInputQueueWorkers, size_t numKernelQueueWorkers, size_t numReadQueueWorkers,
                    const char *kernelDir, unsigned int platformId, unsigned int deviceId, int maxOutstandingKernels,
                    size_t contexts, size_t *localWorkgroupSize)
            : inputQueue(), inputQueueWorkers(), kernelQueueWorkers(), readQueueWorkers(),
              outstandingKernels() {
        inputQueue = std::make_shared<moodycamel::BlockingConcurrentQueue<DispatcherTask>>();


        for (size_t i = 0; i < contexts; i++) {
            auto kernelQueue = std::make_shared<moodycamel::BlockingConcurrentQueue<KernelTask>>();
            auto readQueue = std::make_shared<moodycamel::BlockingConcurrentQueue<ReadTask>>();
            cl_context ctx = OCLExecutor::createContext(platformId);

            for (size_t i = 0; i < numInputQueueWorkers; i++) {
                std::shared_ptr<OCLExecutor> oclExecutor = std::make_unique<OCLExecutor>(
                        kernelDir, platformId, deviceId, localWorkgroupSize, ctx);
                auto worker = std::make_unique<InputQueueWorker>(inputQueue, kernelQueue, oclExecutor,
                                                                 &outstandingKernels, maxOutstandingKernels);
                worker->start();
                inputQueueWorkers.push_back(std::move(worker));
            }

            for (size_t i = 0; i < numKernelQueueWorkers; i++) {
                std::shared_ptr<OCLExecutor> oclExecutor = std::make_unique<OCLExecutor>(
                        kernelDir, platformId, deviceId, localWorkgroupSize, ctx);
                auto worker = std::make_unique<KernelQueueWorker>(kernelQueue, readQueue, oclExecutor,
                                                                  &outstandingKernels);
                worker->start();
                kernelQueueWorkers.push_back(std::move(worker));
            }

            for (size_t i = 0; i < numReadQueueWorkers; i++) {
                std::shared_ptr<OCLExecutor> oclExecutor = std::make_unique<OCLExecutor>(
                        kernelDir, platformId, deviceId, localWorkgroupSize, ctx);
                auto worker = std::make_unique<ReadQueueWorker>(readQueue, oclExecutor);
                worker->start();
                readQueueWorkers.push_back(std::move(worker));
            }
        }
    }

    ~BatchDispatcher() {
        for (size_t i = 0; i < inputQueueWorkers.size(); i++) {
            inputQueueWorkers[i]->stop();
            DispatcherTask stopTask;
            stopTask.empty = true;
        }
        for (size_t i = 0; i < kernelQueueWorkers.size(); i++) {
            kernelQueueWorkers[i]->stop();
            KernelTask stopTask;
            stopTask.empty = true;
        }
        for (size_t i = 0; i < readQueueWorkers.size(); i++) {
            readQueueWorkers[i]->stop();
            ReadTask stopTask;
            stopTask.empty = true;
        }
    }

    void dispatch(DispatcherTask task) {
        inputQueue->enqueue(task);
    }

private:
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<DispatcherTask>> inputQueue;
    std::vector<std::unique_ptr<InputQueueWorker>> inputQueueWorkers;
    std::vector<std::unique_ptr<KernelQueueWorker>> kernelQueueWorkers;
    std::vector<std::unique_ptr<ReadQueueWorker>> readQueueWorkers;

    std::atomic<int> outstandingKernels;
};


#endif //GPUDNS_BATCHDISPATCHER_H
