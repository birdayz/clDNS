#ifndef GPUDNS_READQUEUEWORKER_H
#define GPUDNS_READQUEUEWORKER_H

#include "BatchDispatcher.h"
#include "OCLExecutor.h"
#include <thread>

class ReadQueueWorker {
public:
    ReadQueueWorker(std::shared_ptr<moodycamel::BlockingConcurrentQueue<ReadTask>> readQueue,
                    std::shared_ptr<OCLExecutor> oclExecutor) : readQueue(readQueue), oclExecutor(oclExecutor),
                                                                killSwitch(false) {

    }

    void start() {
        std::thread tr(&ReadQueueWorker::doWork, this);
        tr.detach();
    }

    void stop() {
        killSwitch = true;
    }

private:
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<ReadTask>> readQueue;
    std::shared_ptr<OCLExecutor> oclExecutor;
    std::atomic<bool> killSwitch;

    void doWork() {
        while (!killSwitch) {
            ReadTask task;
            readQueue->wait_dequeue(task);
            auto batchResults = oclExecutor->readResult(task.bufferDigest, task.batchSize);
            task.callback(std::move(batchResults));
        }
    }
};

#endif //GPUDNS_READQUEUEWORKER_H
