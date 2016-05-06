#ifndef GPUDNS_HELPER_H
#define GPUDNS_HELPER_H

#include <sstream>
#include <string>
#include <iostream>
#include <cstring>
#include <array>
#include <CL/cl.h>
#include "batch.h"
#include <functional>
#include <memory>
#include <chrono>



typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<float, std::milli> duration;

typedef unsigned char byte;
typedef std::array<byte, 32> NSEC3HashB32;
typedef std::basic_string<unsigned char> qname;
typedef std::basic_string<unsigned char> NSEC3Salt;

inline void print_u_char(const unsigned char *array, long size) {
    printf("\n");
    int i;
    for (i = 0; i < size; i++) {
        printf("%u:", array[i]);
    }
    printf("\n");
}

inline qname qnameWireFormatToString(unsigned char *qnamePtr) {
    unsigned long domainLength = 0;

    unsigned char *cursor = qnamePtr;

    while (*cursor != 0) {
        domainLength += *cursor + 1;
        cursor += *cursor + 1;
    }
    domainLength = domainLength + 1; // don't forget the trailing 0
    return qname(qnamePtr, domainLength);
}

inline std::string wireToString(qname name) {
    std::string result = std::string((const char *) name.c_str());
    for (unsigned int i = 0; i < result.size(); i++) {
        if (result[i] >= 0 && result[i] < 32) {
            result[i] = '.';
        }
    }
    return result;
}

inline qname textToQname(const char *text) {
    int size = strlen(text);
    unsigned char ret[size + 2];
    unsigned int retIndex = 0;

    unsigned int startCopyIndex = 0;

    unsigned int charsToCopy = 0;

    for (int i = 0; i <= size; i++) {

        if (text[i] == '.' || i == size) {
            //write amount of chars
            ret[retIndex] = charsToCopy;
            retIndex++;

            //write chars
            memcpy(&ret[retIndex], &text[startCopyIndex], charsToCopy);
            retIndex += charsToCopy;

            //reset chars to copy counter
            charsToCopy = 0;
            startCopyIndex = i + 1;
        }
        else {
            charsToCopy++;
        }
    }

    ret[size+1] = '\0';
    return qname(ret, size + 2);
}

inline NSEC3HashB32 textToHash(const char *text) {
    NSEC3HashB32 hash;
    for (unsigned int i=0;i<32;i++) {
        hash[i] = (unsigned char)text[i];
    }
    return hash;
}

inline NSEC3Salt textToSalt(const char *src) {
    size_t destSize = strlen(src)/2;
    unsigned char buf[128];

    unsigned char *dst = buf;
    unsigned char *end = buf + sizeof(buf);
    unsigned int u;

    while (dst < end && sscanf(src, "%2x", &u) == 1)
    {
        *dst++ = u;
        src += 2;
    }
    return NSEC3Salt(buf,destSize);
}

typedef struct dataInfoItem {
    unsigned int dataLengthGlobal;
    unsigned int saltLengthGlobal;
    unsigned int iterations;
    unsigned int itemDomainLength;
    unsigned int itemSaltLength;
} infos_struct;

typedef struct batch {
    unsigned char *domains;
    unsigned char *salts;
    dataInfoItem *dataInfo;
    size_t size;
} batch;





#endif //GPUDNS_HELPER_H
