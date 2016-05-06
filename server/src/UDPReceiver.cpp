#include "UDPReceiver.h"

#include <fstream>
#include <thread>

std::unique_ptr<DNS_REQUEST> UDPReceiver::receive() {
    ssize_t messageLength;
    socklen_t cli_size;
    struct sockaddr_in cliAddr;
    char receiveBuffer[255];

    memset(receiveBuffer, 0, BUF);

    cli_size = sizeof(cliAddr);
    messageLength = sock->receive(receiveBuffer, BUF,
                                 &cliAddr, &cli_size);

    if (messageLength < 0) {
        return nullptr;
    } else if (messageLength == 0) {
        return nullptr;
    }
    return this->processPacket(receiveBuffer, messageLength, cliAddr, cli_size);
}

std::unique_ptr<DNS_REQUEST> UDPReceiver::processPacket(char *data, int len, sockaddr_in clientAddr, socklen_t clientAddrLen) {
    std::unique_ptr<DNS_REQUEST> req = std::make_unique<DNS_REQUEST>();

    req->clientAddress = clientAddr;
    req->clientAddressLength = clientAddrLen;

    struct DNS_HEADER *dns = reinterpret_cast<DNS_HEADER *>(data);

    req->id = ntohs(dns->id);

    short numQuestions = ntohs(dns->q_count);

    unsigned char *ptr = (unsigned char *) &data[sizeof(struct DNS_HEADER)];

    // Question section
    for (int i = 0; i < numQuestions; i++) {
        qname name = qnameWireFormatToString(ptr);
        req->domain = std::move(name);
        ptr += req->domain.size() + (sizeof(unsigned short) * 2);
    }

    // TODO we expect that there is no answers section and auth. servers section

    //additional sec
    R_DATA rr_fields;
    unsigned char domainRoot;

    parseAdditionalSection(ptr, &domainRoot, &rr_fields);

    EDNS_TTL *edns_ttl = reinterpret_cast<EDNS_TTL *>(&rr_fields.ttl);

    if (edns_ttl->do_flag == 1) {
        req->dnssec = true;
    }
    return req;
}

int UDPReceiver::parseAdditionalSection(unsigned char *ptr, unsigned char *domain, R_DATA *rr_fields) {
    int domainSize = parseName(ptr, domain);
    memcpy(rr_fields, ptr + domainSize, sizeof(R_DATA));

    return domainSize;
}

int UDPReceiver::parseName(unsigned char *namePtr, unsigned char *outputName) {
    int domainLength = 0;
    //unsigned char *startPtr = namePtr;
    while (*namePtr != 0) {
        domainLength += *namePtr + 1;
        namePtr += *namePtr + 1;
    }
    domainLength = domainLength + 1; // don't forget the trailing 0
    memcpy(outputName, namePtr - (domainLength) + 1, domainLength);
    return domainLength;
}


UDPReceiver::UDPReceiver(std::shared_ptr<UDPSocket> socket, NameLookupProcessor &nl) :
        sock(socket), nl(nl), mainThreadExitFlag(false), mutex(), workerFinished(false), signal() {
}

void UDPReceiver::stop() {
    sock->stop();
}

UDPReceiver::~UDPReceiver() {
    sock->stop();
}

