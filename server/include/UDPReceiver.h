#ifndef GPUDNS_UDPRECEIVER_H
#define GPUDNS_UDPRECEIVER_H

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <netinet/in.h>
#include <memory>

#include "UDPSocket.h"
#include "udpPacket.h"
#include "NameLookupProcessor.h"

class UDPReceiver {
public:
    UDPReceiver(std::shared_ptr<UDPSocket> socket, NameLookupProcessor &nl);

    ~UDPReceiver();

    std::unique_ptr<DNS_REQUEST> receive();

    void stop();

private:
    std::shared_ptr<UDPSocket> sock;
    NameLookupProcessor &nl;

    std::atomic<bool> mainThreadExitFlag;
    std::mutex mutex;
    bool workerFinished;
    std::condition_variable signal;


    inline std::unique_ptr<DNS_REQUEST> processPacket(char *data, int len, sockaddr_in clientAddr, socklen_t clientAddrLen);

    inline int parseName(unsigned char *namePtr, unsigned char *outputName);

    int parseAdditionalSection(unsigned char *ptr, unsigned char *domain, R_DATA *rr_fields);
};

#endif
