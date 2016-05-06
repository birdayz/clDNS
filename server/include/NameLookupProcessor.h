#ifndef GPUDNS_NAMELOOKUPPROCESSOR_H
#define GPUDNS_NAMELOOKUPPROCESSOR_H

#include "DNSRequest.h"
#include <memory>
#include "UDPSender.h"
#include "ZoneManager.h"
#include "NSEC3Processor.h"

/**
 * Processes a name lookup request
 */
class NameLookupProcessor {
public:
    NameLookupProcessor(ZoneManager &zoneManager, NSEC3Processor &nsec3Processor);

    ~NameLookupProcessor();

    std::unique_ptr<DNS_REQUEST> process(std::unique_ptr<DNS_REQUEST> request);

private:
    Zone *findZone(unsigned char *domain);

    ZoneManager &zoneManager;
    NSEC3Processor &nsec3Processor;
};


#endif //GPUDNS_NAMELOOKUPPROCESSOR_H
