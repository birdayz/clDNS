#ifndef GPUDNS_BASE32HEX_H
#define GPUDNS_BASE32HEX_H

#include <string>
#include "LDNSWrapper.h"
#include <array>

int fromBase32Hex(NSEC3HashB32 input, unsigned char *output);
std::array<unsigned char, 32> toBase32Hex(unsigned char*input);

#endif //GPUDNS_BASE32HEX_H
