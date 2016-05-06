#include <include/udpPacket.h>
#include "UDPSender.h"
#include <algorithm>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;


UDPSender::UDPSender(std::shared_ptr<UDPSocket> udpSocket) : udpSocket(udpSocket) {

}

UDPSender::~UDPSender() {

}

size_t UDPSender::serializeRequest(const DNS_REQUEST &req, unsigned char *buf) {
    size_t packetSize = 0;
    //header
    DNS_HEADER *header = reinterpret_cast<DNS_HEADER *>(buf);
    header->id = htons(req.id);
    header->qr = 1;
    header->opcode = 0;
    header->aa = 1;
    header->tc = 0;
    header->rd = 0;
    header->ra = 0;
    header->cd = 0;
    header->ad = 0;

    header->z = 0;
    header->rcode = 0;
    header->q_count = 0;

    // end header

    packetSize += sizeof(DNS_HEADER);
    unsigned char *cursor = buf + sizeof(DNS_HEADER);

    // answer section
    unsigned short ans_count = 0;
    for (const ResourceRecord *record : req.answers) {
        ans_count++;
        size_t bytesWritten = record->serialize(cursor);
        packetSize += bytesWritten;
        cursor += bytesWritten;
    }

    unsigned short auth_count = 0;

    //AUTH SEC
    for (int i = 0; i < 3; i++) {
        if (req.nsec3Records[i] != nullptr) { // FIXME not necessary if getenclosing record
            auth_count++;

            size_t bytesWritten = req.nsec3Records[i]->serialize(cursor);
            packetSize += bytesWritten;
            cursor += bytesWritten;

            size_t bytesWrittenRRSig = req.nsec3Records[i]->getRRSig()->serialize(cursor);

            packetSize += bytesWrittenRRSig;
            cursor += bytesWrittenRRSig;
            auth_count++;
        }
    }

    unsigned short add_count = 0;
    PseudoRecord ps = PseudoRecord(1200); //TODO
    size_t bytes_written = ps.serialize(cursor);
    packetSize += bytes_written;
    cursor += bytes_written;
    add_count++;

    header->add_count = htons(add_count); // TODO
    header->ans_count = htons(ans_count);
    header->auth_count = htons(auth_count);

    return packetSize;
}

void UDPSender::writeResponse(std::unique_ptr<DNS_REQUEST> req) {
    unsigned char buf[1408];
    // Remove duplicates
    if (req->nsec3Records[1] != nullptr && req->nsec3Records[1] == req->nsec3Records[0]) {
        req->nsec3Records[1] = nullptr;
    }
    if (req->nsec3Records[2] != nullptr && ((req->nsec3Records[2] == req->nsec3Records[0]) ||
                                            (req->nsec3Records[2] == req->nsec3Records[1]))) {
        req->nsec3Records[2] = nullptr;
    }

    size_t len = serializeRequest(*req.get(), &buf[0]);
    ssize_t result = udpSocket->sendTo((char *) buf, len, &req->clientAddress, req->clientAddressLength);
    if (result == -1) {
        std::cout << "error on send" << std::endl;
    }
}

void UDPSender::process_batch(std::vector<std::unique_ptr<DNS_REQUEST>>::iterator start,
                   std::vector<std::unique_ptr<DNS_REQUEST>>::iterator end) {


    mmsghdr *messages = new mmsghdr[1024];
    unsigned char *buf = new unsigned char[1408 * 1024];
    iovec *iovecs = new iovec[1024];

    int i=0;
    while (start != end) {
        msghdr message;
        message.msg_name = static_cast<void *>(&start->get()->clientAddress);
        message.msg_namelen = start->get()->clientAddressLength;


        size_t len = serializeRequest(*start->get(), &buf[i * 1408]);
        iovecs[i].iov_base = &buf[i * 1408];
        iovecs[i].iov_len = len;
        message.msg_iov = &iovecs[i];
        message.msg_iovlen = 1;
        messages[i].msg_hdr = message;
        messages[i].msg_len = 1;
        start++;
        i++;
    }

    int num = udpSocket->sendMmsg(&messages[0], (unsigned int) i, 0);
    assert(num == i);
    delete[] messages;
    delete[] buf;
    delete[] iovecs;

}