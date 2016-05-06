#ifndef CLDNS_HASH_QUEUE_H
#define CLDNS_HASH_QUEUE_H

#include "DNSRequest.h"
#include "helper.h"
#include <tuple>
#include "blockingconcurrentqueue.h"
#include <chrono>
#include <thread>
#include "Zone.h"
#include <condition_variable>
#include <mutex>

typedef std::chrono::high_resolution_clock Clock;

class HashRequest {
public:


    HashRequest(std::unique_ptr<DNS_REQUEST> request, const qname domainToHash, const Zone *zone) : request(
            std::move(request)),
                                                                                                    domainToHash(
                                                                                                            domainToHash),
                                                                                                    zone(zone) {

    }

    ~HashRequest() {

    }

    HashRequest() {

    }

    std::unique_ptr<DNS_REQUEST> request;
    qname domainToHash;
    const Zone *zone;

    HashRequest(HashRequest &&other) : request(std::move(other.request)), domainToHash(other.domainToHash),
                                       zone(other.zone) {

    }

    HashRequest &operator=(HashRequest &&other) {
        request = std::move(other.request);
        domainToHash = std::move(other.domainToHash);
        zone = other.zone;
        return *this;
    }

};

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>


template<typename T, size_t SIZE>
class HashQueue {
public:

    std::pair<std::shared_ptr<std::array<T, SIZE>>, int> pop() {

        auto ret = std::make_shared<std::array<T, SIZE>>();
        std::unique_lock<std::mutex> lock(m);
        while (numEnqueuedItems < BATCH_SIZE) {  // loop to avoid spurious wakeups
            bool timedOut = (std::cv_status::timeout == cond_var.wait_for(lock, std::chrono::milliseconds(100)));
            if (timedOut && numEnqueuedItems > 0) {
                break;
            }
        }

        size_t count = q.try_dequeue_bulk(ret->data(), SIZE);
        numEnqueuedItems = numEnqueuedItems - count;
        return std::make_pair(ret, count);
    }

    void push(T &&item) {
        std::unique_lock<std::mutex> lock(m);
        auto success = q.enqueue(std::move(item));
        if (success) {
            numEnqueuedItems++;
            if (numEnqueuedItems == BATCH_SIZE) {
                cond_var.notify_one();
            }

        }
    }

    HashQueue() : q(), numEnqueuedItems(0) {

    };

    ~HashQueue() {

    }

    HashQueue(const HashQueue &) = delete;            // disable copying
    HashQueue &operator=(const HashQueue &) = delete; // disable assignment

private:
    moodycamel::BlockingConcurrentQueue<T> q;
    std::mutex m;
    std::condition_variable cond_var;
    size_t numEnqueuedItems;
};

#endif

