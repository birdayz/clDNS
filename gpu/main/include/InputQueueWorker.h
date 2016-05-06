#ifndef GPUDNS_INPUTQUEUEWORKER_H
#define GPUDNS_INPUTQUEUEWORKER_H

#include "BatchDispatcher.h"
#include "OCLExecutor.h"

class InputQueueWorker {
public:
    InputQueueWorker(std::shared_ptr<moodycamel::BlockingConcurrentQueue<DispatcherTask>> inputQueue,
                     std::shared_ptr<moodycamel::BlockingConcurrentQueue<KernelTask>> kernelQueue,
                     std::shared_ptr<OCLExecutor> oclExecutor, std::atomic<int> *outstandingKernels,
                     int maxOutstandingKernels) : inputQueue(
            inputQueue),
                                                  kernelQueue(
                                                          kernelQueue),
                                                  oclExecutor(
                                                          oclExecutor),
                                                  killSwitch(
                                                          false),
                                                  outstandingKernels(
                                                          outstandingKernels),
                                                  maxOutstandingKernels(maxOutstandingKernels) {

    }

    void start() {
        std::thread tr(&InputQueueWorker::doWork, this);
        tr.detach();
    }

    void stop() {
        killSwitch = true;
    }

    void direct(DispatcherTask task) {
        auto kernelTask = oclExecutor->writeToDevice(task.data_info, task.domain, task.salt, task.domainLength,
                                                     task.saltLength, task.batchSize);
        kernelTask.callback = task.callback;
        kernelQueue->enqueue(kernelTask);
    }

private:
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<DispatcherTask>> inputQueue;
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<KernelTask>> kernelQueue;
    std::shared_ptr<OCLExecutor> oclExecutor;
    std::atomic<bool> killSwitch;
    std::atomic<int> *outstandingKernels;
    int maxOutstandingKernels;


    void doWork() {
        while (!killSwitch) {
            DispatcherTask task;
            inputQueue->wait_dequeue(task);
            while (*outstandingKernels >= maxOutstandingKernels) {
                std::this_thread::yield();
            }
            ++*outstandingKernels;

            auto kernelTask = oclExecutor->writeToDevice(task.data_info, task.domain, task.salt, task.domainLength,
                                                         task.saltLength, task.batchSize);
            kernelTask.callback = task.callback;
            kernelQueue->enqueue(kernelTask);
        }
    }
};


#endif //GPUDNS_INPUTQUEUEWORKER_H
