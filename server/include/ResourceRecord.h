#ifndef GPUDNS_RESOURCERECORD_H
#define GPUDNS_RESOURCERECORD_H

#include <string>
#include <memory>
#include <vector>
#include <iterator>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <array>
#include <vector>
#include "helper.h"
#include "udpPacket.h"
#include "LDNSWrapper.h"
#include "spdlog/spdlog.h"
#include "Base32Hex.h"

#define IPV4_RDATA_LENGTH 4

enum RRType {
    PSEUDO = 0,
    A = 1,
    NS = 2,
    MD = 3,
    MF = 4,
    CNAME = 5,
    SOA = 6,
    MB = 7,
    MG = 8,
    MR = 9,
    NULL_ = 10,
    WKS = 11,
    PTR = 12,
    HINFO = 13,
    MINFO = 14,
    MX = 15,
    TXT = 16,
    RRSIG = 46,
    NSEC3 = 50
};
enum RRclazz {
    IN
};

class ResourceRecord {
public:
    /**
    * @param out pointer to the buffer this objects is serialized to
    * @return number of bytes written
    */
    virtual size_t serialize(unsigned char *out) const = 0;

    inline const qname &getName() const {
        return name;
    }

    inline RRType getRRType() const {
        return type;
    }

    static const std::shared_ptr<spdlog::logger> log;
protected:
    ResourceRecord(qname name, unsigned int ttl, RRclazz clazz, RRType type) : name(name),
                                                                      ttl(ttl),
                                                                      clazz(clazz),
                                                                      type(type) { }

    qname name;
    unsigned int ttl;
    RRclazz clazz;
    RRType type;

    //std::vector<unsigned char> rdata;
    //std::unique_ptr<unsigned char *> rdata;
};

/**
 *  The RDATA of the NSEC3 RR is as shown below:

                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Hash Alg.   |     Flags     |          Iterations           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Salt Length  |                     Salt                      /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Hash Length  |             Next Hashed Owner Name            /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /                         Type Bit Maps                         /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Hash Algorithm is a single octet.

   Flags field is a single octet, the Opt-Out flag is the least
   significant bit, as shown below:

    0 1 2 3 4 5 6 7
   +-+-+-+-+-+-+-+-+
   |             |O|
   +-+-+-+-+-+-+-+-+
 */

#pragma pack(push, 1)
struct NSEC3_RDATA {
    unsigned char hashAlg;
    unsigned char flags;
    unsigned short iterations;
};
#pragma pack(pop)

class RRSigRecord : public ResourceRecord {
public:
    RRSigRecord(qname name, unsigned int ttl, RRclazz clazz, unsigned char numLabelsOwner, unsigned short keyTag,
                unsigned int expirationDate, unsigned int signatureInception, const qname &signerName,
                std::vector<unsigned char> signature)
            : ResourceRecord(name, ttl, clazz,
                             RRType::RRSIG),
              labels(numLabelsOwner), originalTTL(ttl), expirationDate(expirationDate),
              signatureInception(signatureInception), keyTag(keyTag), signerName(signerName),
              signature(std::move(signature)) { }

    RRSigRecord(RRSigRecord &&other) : ResourceRecord(std::move(other.name), other.ttl, other.clazz, other.type),
                                       labels(other.labels), originalTTL(other.originalTTL),
                                       expirationDate(other.expirationDate),
                                       signatureInception(other.signatureInception), keyTag(other.keyTag),
                                       signerName(other.signerName), signature(std::move(other.signature)) { }

    size_t serialize(unsigned char *out) const;

    ~RRSigRecord() { }

private:
    //RRType typeCovered; // TODO unused
    //unsigned char algorithm; // TODO unused

    /**
     3.1.3.  The Labels Field
     * For example,
     "www.example.com." has a Labels field value of 3, and
     "*.example.com." has a Labels field value of 2.  Root (".") has a
     Labels field value of 0.
     */
    unsigned char labels; // TODO !! in constructor setzen
    int originalTTL; //TODO maybe take ttl from original record?
    unsigned int expirationDate;
    unsigned int signatureInception;
    unsigned short keyTag;
    qname signerName;
    std::vector<unsigned char> signature;
};

class NSEC3Record : public ResourceRecord {
public:

    NSEC3Record(qname name, unsigned int ttl, RRclazz clazz, unsigned short iterations, NSEC3Salt salt, NSEC3HashB32 nextHash,
                std::unique_ptr<RRSigRecord> rrSigRecord)
            : ResourceRecord(
            name, ttl, clazz,
            RRType::NSEC3), iterations(iterations), salt(salt), nextHash(nextHash),
              rrSigRecord(std::move(rrSigRecord)) { }

    size_t serialize(unsigned char *out) const;

    ~NSEC3Record() { }

    NSEC3Record(NSEC3Record &&other) : ResourceRecord(std::move(other.name), other.ttl, other.clazz, other.type),
                                       iterations(other.iterations), salt(std::move(other.salt)),
                                       nextHash(std::move(other.nextHash)),
                                       rrSigRecord(std::move(other.rrSigRecord)) { }

    NSEC3Record &operator=(NSEC3Record &&other) {
        name = std::move(other.name);
        ttl = other.ttl;
        clazz = other.clazz;
        type = other.type;
        iterations = other.iterations;
        salt = std::move(other.salt);
        nextHash = std::move(other.nextHash);

        return *this;
    }

    inline const NSEC3HashB32 &getHash() const {
        return nextHash;
    }

    inline const NSEC3Salt &getSalt() const {
        return salt;
    }

    const RRSigRecord *getRRSig() const {
        return rrSigRecord.get();
    }

private:
    unsigned short iterations;
    NSEC3Salt salt;
    NSEC3HashB32 nextHash;
    std::unique_ptr<RRSigRecord> rrSigRecord;
};

/**
 * RRSIG RR Wire Format
 *
 *                      1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |        Type Covered           |  Algorithm    |     Labels    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Original TTL                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                      Signature Expiration                     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                      Signature Inception                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |            Key Tag            |                               /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+         Signer's Name         /
   /                                                               /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /                                                               /
   /                            Signature                          /
   /                                                               /
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

 */
#pragma pack(push, 1)
struct RRSIG_RDATA {
    unsigned short typeCovered;
    unsigned char algorithm;
    unsigned char labels;
    unsigned int originalTTL;
    unsigned int signatureExpiration;
    unsigned int signatureInception;
    unsigned short keyTag;
};
#pragma pack(pop)


class ARecord : public ResourceRecord {
public:
    ARecord(qname name, unsigned int ttl, RRclazz clazz, unsigned char *ip) : ResourceRecord(
            name, ttl, clazz,
            RRType::A), ip(ip, ip + 4) { }

    ARecord(ARecord &&other) : ResourceRecord(std::move(other.name), other.ttl, other.clazz, other.type),
                               ip(std::move(other.ip)) { }

    size_t serialize(unsigned char *out) const;

    ~ARecord() { }

private:
    std::vector<unsigned char> ip;
};

static unsigned char bla[1] = { 0 };
static qname emptyQname = qname(bla,1); // hacky hack

class PseudoRecord : public ResourceRecord {
public:
    PseudoRecord(unsigned short udpPayloadSize) : ResourceRecord(emptyQname, 0, RRclazz::IN,
            RRType::PSEUDO), udpPayloadSize(udpPayloadSize) {
        EDNS_TTL *edns_ttl = reinterpret_cast<EDNS_TTL *>(&ttl);
        edns_ttl->extendedRcode = 0;
        edns_ttl->version = 0;
        edns_ttl->do_flag = 1;
        edns_ttl->z = 0;
        edns_ttl->z_remaining = 0;
    }

    PseudoRecord(PseudoRecord &&other) : ResourceRecord(std::move(other.name), other.ttl, other.clazz, other.type) { }

    size_t serialize(unsigned char *out) const;

    ~PseudoRecord() {

    }

private:
    unsigned short udpPayloadSize;
};

inline const NSEC3Record* getCoveringRecord(const std::vector<const NSEC3Record*> &nsec3Chain, NSEC3HashB32 domainHash) {

    if (domainHash < nsec3Chain.back()->getHash() ||
        memcmp(domainHash.data(), nsec3Chain.back()->getName().c_str() + 1, 32) > 0) { // smaller than
        return nsec3Chain[nsec3Chain.size() - 1];
    }
    for (const NSEC3Record *nsec3RR : nsec3Chain) {
        NSEC3HashB32 nsec3RRQnameHashPart;
        memcpy(nsec3RRQnameHashPart.data(), nsec3RR->getName().c_str() + 1, 32);

        if (nsec3RRQnameHashPart < domainHash && nsec3RR->getHash() > domainHash) {

            return nsec3RR;
        }

    }
    return nullptr;
}

#endif //GPUDNS_RESOURCERECORD_H
