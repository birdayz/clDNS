#ifndef GPUDNS_UDPSENDER_H
#define GPUDNS_UDPSENDER_H

#include "DNSRequest.h"
#include "UDPSocket.h"
#include <memory>



class UDPSender {
public:
    UDPSender(std::shared_ptr<UDPSocket> udpSocket);

    ~UDPSender();

    /**
     * Serializes response
     */
    void writeResponse(std::unique_ptr<DNS_REQUEST> req);

    void writeResponseBatch(std::vector<std::unique_ptr<DNS_REQUEST>> reqs) {

        size_t batchSize = 1024;
        auto start = reqs.begin();
        auto next = start + batchSize;

        while (next < reqs.end()) {
            process_batch(start, next);
            start = next;
            next += batchSize;
        }
        process_batch(start, reqs.end());

    }

private:
    std::shared_ptr<UDPSocket> udpSocket;

    void process_batch(std::vector<std::unique_ptr<DNS_REQUEST>>::iterator start,
                       std::vector<std::unique_ptr<DNS_REQUEST>>::iterator end);

    // return actual length
    size_t serializeRequest(const DNS_REQUEST &req, unsigned char *buf);
};


#endif //GPUDNS_UDPSENDER_H
