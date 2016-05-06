#ifndef GPUDNS_DNSREQUEST_H
#define GPUDNS_DNSREQUEST_H

#include <bitset>
#include <netinet/in.h>
#include "helper.h"
#include <vector>
#include "ResourceRecord.h"

typedef unsigned char dnsreq_status;


struct DNS_REQUEST {
    unsigned short id;
    sockaddr_in clientAddress;
    socklen_t clientAddressLength;
    qname domain;
    bool dnssec = false;

    // Response records
    std::vector<const ResourceRecord *> answers;
    std::vector<const ResourceRecord *> authorityAnswers;
    std::array<const NSEC3Record *,3> nsec3Records;
};

#endif //GPUDNS_DNSREQUEST_H
