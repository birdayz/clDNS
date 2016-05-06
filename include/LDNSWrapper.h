#ifndef GPUDNS_LDNSWRAPPER_H
#define GPUDNS_LDNSWRAPPER_H

#include <string>
#include <ldns/ldns.h>
#include "helper.h"
#include <array>
#include <memory>

#define NSEC3_HASH_B32_LENGTH 32
#define NSEC3_HASH_ALGORITHM 1






inline NSEC3HashB32 NSEC3HashB32fromCString(const char *string) {
    NSEC3HashB32 ret;
    memcpy(ret.data(), string, 32);
    return ret;
}

class Nsec3HashB32Functor {
public:
    struct structHashDeleter {
        void operator()(ldns_rdf *hash) {
            ldns_rdf_deep_free(hash);
        }
    };

    __attribute__((optimize("unroll-loops")))
    inline NSEC3HashB32 operator()(const qname &name, const NSEC3Salt &salt, const short unsigned iterations) {
        ldns_rdf d_name;
        d_name._data = const_cast<unsigned char *>(name.c_str());
        d_name._size = name.size();
        d_name._type = LDNS_RDF_TYPE_A;

        auto hash = std::unique_ptr<ldns_rdf, structHashDeleter>(ldns_nsec3_hash_name(&d_name,
                                                                                      NSEC3_HASH_ALGORITHM,
                                                                                      iterations,
                                                                                      static_cast<uint8_t>(salt.size()),
                                                                                      const_cast<unsigned char *>(salt.c_str())));

        NSEC3HashB32 hashRet;

        memcpy(hashRet.data(), (unsigned char *) hash->_data + 1, 32);
        for (unsigned int i = 0; i < NSEC3_HASH_B32_LENGTH; i++) {
            if (hashRet[i] >= 97 && hashRet[i] < 123) {
                hashRet[i] = hashRet[i] - 32;
            }
        }
        return hashRet;
    }

    __attribute__((optimize("unroll-loops")))
    inline NSEC3HashB32 operator()(const unsigned char *name, const size_t nameLength, const NSEC3Salt &salt,
                                   const short unsigned iterations) {
        ldns_rdf d_name;
        d_name._data = const_cast<unsigned char *>(name);
        d_name._size = nameLength;
        d_name._type = LDNS_RDF_TYPE_A;

        auto hash = std::unique_ptr<ldns_rdf, structHashDeleter>(ldns_nsec3_hash_name(&d_name,
                                                                                      NSEC3_HASH_ALGORITHM,
                                                                                      iterations,
                                                                                      static_cast<uint8_t>(salt.size()),
                                                                                      const_cast<unsigned char *>(salt.c_str())));

        NSEC3HashB32 hashRet;

        memcpy(hashRet.data(), (unsigned char *) hash->_data + 1, 32);
        for (unsigned int i = 0; i < NSEC3_HASH_B32_LENGTH; i++) {
            if (hashRet[i] >= 97 && hashRet[i] < 123) {
                hashRet[i] = hashRet[i] - 32;
            }
        }
        return hashRet;
    }
};

#endif //GPUDNS_LDNSWRAPPER_H
