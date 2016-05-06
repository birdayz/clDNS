#include "PacketWorker.h"

void PacketWorker::doWork() {
    while (!abort) {
        std::unique_ptr<DNS_REQUEST> req = udpReceiver->receive();
        if (req != nullptr) {
            std::unique_ptr<DNS_REQUEST> processedRequest = nameLookupProcessor.process(std::move(req));
            if (processedRequest != nullptr)
                udpSender->writeResponse(std::move(processedRequest));
        }
    }
}

