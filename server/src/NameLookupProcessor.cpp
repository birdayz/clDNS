#include <iostream>
#include <chrono>
#include "NameLookupProcessor.h"

NameLookupProcessor::NameLookupProcessor(ZoneManager &zoneManager, NSEC3Processor &nsec3Processor)
        : zoneManager(zoneManager), nsec3Processor(nsec3Processor) {

}

NameLookupProcessor::~NameLookupProcessor() {

}

std::unique_ptr<DNS_REQUEST> NameLookupProcessor::process(std::unique_ptr<DNS_REQUEST> request) {
    unsigned char *seek = const_cast<unsigned char *>(request->domain.c_str());
    assert(*seek != 0);

    // find zone
    const Zone *z = nullptr;
    size_t charsToSkip = 0;
    while (*seek != 0) {
        z = zoneManager.getZone(seek, (request->domain.size() - charsToSkip));
        size_t bytesToSkip = *seek + 1;
        charsToSkip += bytesToSkip;
        seek += bytesToSkip;
        if (z != nullptr) {
            break;
        }
    }

    if (z != nullptr) {
        const ResourceRecordSet *aRecord = z->findRecord(request->domain, RRType::A);
        if (aRecord != nullptr) {
            for (size_t i = 0; i < aRecord->size(); i++) {
                request->answers.push_back(aRecord->get(0)); // FIXME teuer
            }
            request->answers.push_back(aRecord->getRRSigRecord());

        }
        else if (request->dnssec) {
            nsec3Processor.startProcessing(std::move(request), *z);
            return nullptr;
            //     return nsec3Processor.processSync(std::move(request), *z);
        }
        else {
            // do nothing
        }
    }
    else {
        // Zone not found
    }
    return request;
}