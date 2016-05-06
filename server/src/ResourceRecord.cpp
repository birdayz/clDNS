#include <include/LDNSWrapper.h>
#include "ResourceRecord.h"
#include <bitset>

size_t ARecord::serialize(unsigned char *out) const {
    memcpy(out, name.c_str(), name.size());

    out += name.size();

    R_DATA *r_data = reinterpret_cast<R_DATA *>(out);

    r_data->type = htons(1); // A record
    r_data->_class = htons(1); //inet
    r_data->ttl = htonl(ttl); // FIXME ttl is always 4
    r_data->data_len = htons(4); // FIXME

    //proceed cursor

    out += sizeof(R_DATA);

    for (int i = 0; i < IPV4_RDATA_LENGTH; i++) {
        out[i] = ip[i];
    }

    return sizeof(unsigned char) * name.size() + sizeof(R_DATA) +
           (sizeof(unsigned char) * IPV4_RDATA_LENGTH);
}


size_t NSEC3Record::serialize(unsigned char *out) const {
    memcpy(out, name.c_str(), name.size());

    out += name.size();

    R_DATA *r_data = reinterpret_cast<R_DATA *>(out);

    r_data->type = htons(50); // NSEC3 record
    r_data->_class = htons(1); //inet
    r_data->ttl = htonl(ttl);

    int rdataSize = 0;
    out += sizeof(R_DATA);

    NSEC3_RDATA *ptr = reinterpret_cast<NSEC3_RDATA *>(out);
    ptr->hashAlg = 1;
    ptr->flags = 1;
    ptr->iterations = htons(iterations);
    rdataSize += sizeof(NSEC3_RDATA);
    out += sizeof(NSEC3_RDATA);

    assert(salt.size() <= 255);
    *out = static_cast<unsigned char>(salt.size());
    out++;
    rdataSize++;
    memcpy(out, salt.c_str(), salt.size());
    out += salt.size();
    rdataSize += salt.size();

    *out = static_cast<unsigned char>(20);
    out++;
    rdataSize++;

    unsigned char base[20];
    fromBase32Hex(nextHash, &base[0]);
    memcpy(out, base, 20);
    rdataSize += 20;
    out += 20;

    // TODO get bitset from zone/rrset
    out[0] = 0;
    out[1] = 1;
    out[2] = 64;

    rdataSize += 3;
    out += 3;

    r_data->data_len = htons(rdataSize);
    return sizeof(unsigned char) * name.size() + sizeof(R_DATA) +
           (sizeof(unsigned char) * rdataSize);
}

size_t RRSigRecord::serialize(unsigned char *out) const {
    memcpy(out, name.c_str(), name.size());

    out += name.size();

    R_DATA *r_data = reinterpret_cast<R_DATA *>(out);

    r_data->type = htons(46); // RRSIG record
    r_data->_class = htons(1); //inet TODO we only allow inet for now
    r_data->ttl = htonl(ttl);

    int rdataSize = 0;
    out += sizeof(R_DATA);

    RRSIG_RDATA *ptr = reinterpret_cast<RRSIG_RDATA *>(out);
    ptr->typeCovered = htons(1);
    ptr->algorithm = 7; //TODO

    ptr->labels = labels;

    ptr->signatureExpiration = htonl(expirationDate);
    ptr->signatureInception = htonl(signatureInception);
    ptr->keyTag = htons(keyTag);

    out += sizeof(RRSIG_RDATA);
    rdataSize += sizeof(RRSIG_RDATA);

    memcpy(out, signerName.c_str(), signerName.size());

    out += signerName.size();
    rdataSize += signerName.size();

    memcpy(out, signature.data(), signature.size());

    out += signature.size();
    rdataSize += signature.size();

    r_data->data_len = htons(rdataSize);

    return sizeof(unsigned char) * name.size() + sizeof(R_DATA) +
           (sizeof(unsigned char) * rdataSize);
}

size_t PseudoRecord::serialize(unsigned char *out) const {
    memcpy(out, name.c_str(), name.size());

    out += name.size();

    R_DATA *r_data = reinterpret_cast<R_DATA *>(out);

    r_data->type = htons(41); // RRSIG record
    r_data->_class = htons(udpPayloadSize); //inet TODO we only allow inet for now
    r_data->ttl = ttl;
    int rdataSize = 0;
    out += sizeof(R_DATA);

    r_data->data_len = htons(rdataSize);

    return sizeof(unsigned char) * name.size() + sizeof(R_DATA) +
           (sizeof(unsigned char) * rdataSize);
}

const std::shared_ptr<spdlog::logger> ResourceRecord::log = spdlog::stdout_logger_mt("ResourceRecord");