#ifndef GPUDNS_PACKET_WORKER_H
#define GPUDNS_PACKET_WORKER_H

#include <memory>
#include "UDPSocket.h"
#include "UDPSender.h"
#include "UDPReceiver.h"
#include <thread>
#include <iostream>
#include "NameLookupProcessor.h"
#include <string>
#include <atomic>
#include <pthread.h>

class PacketWorker {
public:
    PacketWorker(std::unique_ptr<UDPReceiver> udpReceiver, std::unique_ptr<UDPSender> udpSender,
                 NameLookupProcessor &nameLookupProcessor, int id) : udpReceiver(
            std::move(udpReceiver)), udpSender(std::move(udpSender)), nameLookupProcessor(nameLookupProcessor),
                                                                     abort(false) {


    }

    void start() {
        std::thread tr(&PacketWorker::doWork, this);
        tr.detach();
    }

    void stop() {
        abort = true;
    }

    ~PacketWorker() {

    }

    static std::unique_ptr<PacketWorker> create(NameLookupProcessor &nl, int id,
                                                std::shared_ptr<UDPSocket> udpSocket) {
        std::unique_ptr<UDPReceiver> udpReceiver = std::make_unique<UDPReceiver>(udpSocket, nl);
        std::unique_ptr<UDPSender> udpSender = std::make_unique<UDPSender>(udpSocket);

        return std::make_unique<PacketWorker>(std::move(udpReceiver), std::move(udpSender), nl, id);
    }

private:
    void doWork();

    std::unique_ptr<UDPReceiver> udpReceiver;
    std::unique_ptr<UDPSender> udpSender;
    NameLookupProcessor &nameLookupProcessor;
    std::atomic<bool> abort;
};


#endif //GPUDNS_PACKET_WORKER_H
