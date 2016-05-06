#include "Base32Hex.h"

#define NSEC3_HASH_BINARY_SIZE 20

inline unsigned char map(unsigned char in) {
    if (in >= 48 && in < 58) {
        return in - 48;
    }
    else if (in >= 65 && in < 91) {
        return in - 55;
    }
    return 1;
}

inline unsigned char fetch(unsigned char currentByte, unsigned char offset, unsigned char amount) {
    //std::cout << "Fetch byte: " << (int) currentByte << " offset: " << offset << "amount " << amount << std::endl;
    return (currentByte >> (8 - offset - amount)) & ((1 << (8 - offset)) - 1);
}

__attribute__((optimize("unroll-loops")))
int fromBase32Hex(NSEC3HashB32 input, unsigned char *output) {
    int inputByte = 0;
    int offset = 3;
    int targetByte = 0;

    while (targetByte < NSEC3_HASH_BINARY_SIZE) {
        int bytesFetched = 0;

        unsigned char previous_iteration = 0;
        int previousSize = 0;
        while (bytesFetched < 8) {
            unsigned char mapped = map(input[inputByte]);

            int availableBits = 8 - offset;
            int neededBits = 8 - bytesFetched;
            if (neededBits >= availableBits) { // new byte
                unsigned char shifted = fetch(mapped, offset, availableBits);
                previous_iteration = previous_iteration | (shifted << (8 - previousSize - availableBits));

                inputByte++;
                offset = ((offset + availableBits) % 8) + 3;

                previousSize += availableBits;
                bytesFetched += availableBits;

            }
            else {
                unsigned char shifted = fetch(mapped, offset, neededBits);

                previous_iteration = previous_iteration | (shifted << (8 - previousSize - neededBits));
                offset = (offset + neededBits) % 8;
                bytesFetched += neededBits;
                previousSize += neededBits;
            }
        }
        output[targetByte] = previous_iteration;
        targetByte++;
    }
    return NSEC3_HASH_BINARY_SIZE;
}

// from https://github.com/henryk/tinydnssec/blob/master/base32hex.c
// slightly modified. produce uppercase
#define to_32hex(c) ((c) < 10 ? (c) + '0' : (c) + 'A' - 10)

std::array<unsigned char, 32> toBase32Hex(unsigned char * in) {
    std::array<unsigned char, 32> ret;
    int len = 20;
    unsigned char* out = ret.data();

    int buf = 0, bits = 0;
    unsigned char *x = out;

    while (len-- > 0) {
        buf <<= 8;
        buf |= *in++;
        bits += 8;
        while (bits >= 5) {
            char c = (buf >> (bits - 5)) & 0x1f;
            *x++ = to_32hex(c);
            bits -= 5;
        }
    }
    if (bits > 0) {
        char c = (buf << (5 - bits)) & 0x1f;
        *x++ = to_32hex(c);
    }
    return ret;


};