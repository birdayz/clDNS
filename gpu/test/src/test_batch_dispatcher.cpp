#include <thread>
#include <mutex>
#include <condition_variable>
#include "gtest/gtest.h"
#include "OCLExecutor.h"
#include "testSupport.h"
#include "LDNSWrapper.h"
#include "Base32Hex.h"
#include "BatchDispatcher.h"

typedef std::chrono::high_resolution_clock Clock;

using namespace testing;

void run_test(BatchDispatcher &batchDispatcher, const char *kernelDir, qname domain, NSEC3Salt salt) {

    Nsec3HashB32Functor hashFunctor = Nsec3HashB32Functor();
    auto hashExpected = hashFunctor(domain.c_str(), domain.size(), salt, 10);

    dataInfoItem *data_info = new dataInfoItem[1];

    for (int i = 0; i < 1; i++) {
        data_info[i].dataLengthGlobal = domain.size();
        data_info[i].saltLengthGlobal = salt.size();
        data_info[i].iterations = 10;
        data_info[i].itemDomainLength = domain.size();
        data_info[i].itemSaltLength = salt.size();
    }
    std::mutex mutex;
    std::condition_variable condVar;
    bool done = false;

    DispatcherTask task;
    task.data_info = data_info;
    task.domain = domain.c_str();
    task.salt = salt.c_str();
    task.domainLength = domain.size();
    task.saltLength = salt.size();
    task.batchSize = 1;
    task.callback = [&hashExpected, &done, &condVar, &mutex](
            std::unique_ptr<std::array<std::array<byte, 20>, BATCH_SIZE>> result) {
        auto B32ResultHash = toBase32Hex(result->data()[0].data());
        EXPECT_TRUE(memcmp(hashExpected.data(), B32ResultHash.data(), 32) == 0);
        std::unique_lock<std::mutex> lock(mutex);
        done = true;
        condVar.notify_all();
    };

    batchDispatcher.dispatch(task);
    // wait for signal from callback
    std::unique_lock<std::mutex> lock(mutex);
    while (!done) {
        condVar.wait(lock);
    }
}

TEST(BATCH_DISPATCHER, test_logic) {
    static BatchDispatcher batchDispatcher(1, 1, 1, kernelDir, 0, 0, 10, 1, nullptr); // multiple batch dispatchers in the same program causes problems
    auto salt = textToSalt("5115CBE8100E470A");

    run_test(batchDispatcher, kernelDir, textToQname("test3.dnsgpu.bogus"), salt);
    run_test(batchDispatcher, kernelDir, textToQname("abc.dnsgpu.bogus"), salt);
    run_test(batchDispatcher, kernelDir, textToQname("abc.dnsgpu.bogus"), salt);
    run_test(batchDispatcher, kernelDir, textToQname("e546zrthg.dnsgpu.bogus"), salt);
    run_test(batchDispatcher, kernelDir, textToQname("lol.dnsgpu.bogus"), salt);
    run_test(batchDispatcher, kernelDir, textToQname("xyz.dnsgpu.bogus"), salt);
}