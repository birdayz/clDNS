#ifndef GPUDNS_KERNELQUEUEWORKER_H
#define GPUDNS_KERNELQUEUEWORKER_H

#include "BatchDispatcher.h"
#include "OCLExecutor.h"

class KernelQueueWorker {
public:
    KernelQueueWorker(std::shared_ptr<moodycamel::BlockingConcurrentQueue<KernelTask>> kernelQueue,
                      std::shared_ptr<moodycamel::BlockingConcurrentQueue<ReadTask>> readQueue,
                      std::shared_ptr<OCLExecutor> oclExecutor, std::atomic<int> *outstandingKernels) : kernelQueue(
            kernelQueue),
                                                                                                        readQueue(
                                                                                                                readQueue),
                                                                                                        oclExecutor(
                                                                                                                oclExecutor),
                                                                                                        killSwitch(
                                                                                                                false),
                                                                                                        outstandingKernels(
                                                                                                                outstandingKernels) {

    }

    void start() {
        std::thread tr(&KernelQueueWorker::doWork, this);
        tr.detach();
    }

    void stop() {
        killSwitch = true;
    }

private:
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<KernelTask>> kernelQueue;
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<ReadTask>> readQueue;
    std::shared_ptr<OCLExecutor> oclExecutor;
    std::atomic<bool> killSwitch;
    std::atomic<int> *outstandingKernels;

    void doWork() {
        while (!killSwitch) {
            KernelTask task;
            kernelQueue->wait_dequeue(task);
            auto readTask = oclExecutor->execKernel(task.bufferDataInfo, task.bufferDomain, task.bufferSalt,
                                                    task.bufferDigest, task.batchSize);
            --*outstandingKernels;
            readTask.callback = task.callback;
            readQueue->enqueue(readTask);
        }

    }
};

#endif //GPUDNS_KERNELQUEUEWORKER_H
