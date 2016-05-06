//#pragma OPENCL EXTENSION cl_amd_printf : enable
//#pragma OPENCL EXTENSION cl_intel_printf : enable

typedef struct dataInfoItem {
    unsigned int dataLengthGlobal;
    unsigned int saltLengthGlobal;
    unsigned int iterations;
    unsigned int itemDomainLength;
    unsigned int itemSaltLength;
} infos_struct;

#ifdef __clang__
#ifndef cl_nv_pragma_unroll
#warning "llvm cheat"
#define cl_khr_byte_addressable_store 1
#define __ENDIAN_LITTLE__ 1
#define NVIDIA 1
#endif
#endif

#ifdef cl_khr_byte_addressable_store
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#endif

#ifdef cl_nv_pragma_unroll
#define NVIDIA
#endif

#ifndef __ENDIAN_LITTLE__
#error "Only little endian platforms supported!"
#endif

// miniming padding size in bit
__constant size_t
min_pad = 64;
// exact size of a block in bit that can be processed by our SHA-1 implementation
__constant size_t
block_bits = 512;

/*
* #define K0  0x5A827999
* #define K1  0x6ED9EBA1
* #define K2  0x8F1BBCDC
* #define K3  0xCA62C1D6
*/

#define H1 0x67452301
#define H2 0xEFCDAB89
#define H3 0x98BADCFE
#define H4 0x10325476
#define H5 0xC3D2E1F0

void post_work(__private unsigned int *l_raw_hash, __private unsigned char *l_hex_hash) {
    int j = 0;
    for (int i = 0; i < 5; i++) {
        l_hex_hash[j++] = (unsigned char) ((l_raw_hash[i] >> 24) & 0xFF);
        l_hex_hash[j++] = (unsigned char) ((l_raw_hash[i] >> 16) & 0xFF);
        l_hex_hash[j++] = (unsigned char) ((l_raw_hash[i] >> 8) & 0xFF);
        l_hex_hash[j++] = (unsigned char) (l_raw_hash[i] & 0xFF);
    }
}

unsigned int prepare_data(__private unsigned char *l_data, __private unsigned int l_data_length,
                          __private unsigned char *l_salt, __private unsigned int l_salt_length,
                          __private unsigned int *l_data4, __private unsigned int *l_salt4) {
    size_t remainder = ((l_data_length * 8) + (l_salt_length * 8)) % block_bits;
    // size of padding in bits
    size_t pad_len = block_bits - remainder;

    size_t data4_size = 32;
    size_t salt4_size = 11;

    if (l_data_length == 0) {
        //printf("Error: Empty data\n");
    }

    /*if (l_data_length + l_salt_length > 119) {
        printf("name + salt longer than 119 bytes is currently not supported.\n");
    }

    if (l_salt_length > 35) {
        printf("salt longer than 35 bytes is currently not supported");
    }
*/
    if (pad_len < min_pad) {
        pad_len += block_bits;
    }
    ++pad_len;

    pad_len &= ~7;

    //If salt is not empty
    if (l_salt_length > 0) {
        unsigned long tempsaltlength = l_salt_length * 8 + 160;
        int j = 0;
        int k = 0;
        for (int i = 0; i < salt4_size - 2; i++) {
            l_salt4[i] = 0x00;
            if (k <= l_salt_length) {
                do {
                    if (k < l_salt_length) {
                        l_salt4[i] = l_salt4[i] << 8;
                        l_salt4[i] += l_salt[j++];
                        k++;
                    }
                    else if (k == l_salt_length) {
                        l_salt4[i] = l_salt4[i] << 8;
                        l_salt4[i] += 0x80;
                        k++;
                    }
                    else if (k >= l_salt_length) {
                        l_salt4[i] = l_salt4[i] << 8;
                        k++;
                    }
                } while ((k % 4) != 0);
            }
        }

        l_salt4[9] = (tempsaltlength >> 32) & 0xFFFFFFFF;
        l_salt4[10] = (tempsaltlength) & 0xFFFFFFFF;
    }
    else // if salt is empty
    {
        unsigned long tempsaltlength = 160;
        l_salt4[0] = 0x80000000;
        for (int i = 1; i < salt4_size - 2; i++) {
            l_salt4[i] = 0x00;
        }
        l_salt4[9] = (tempsaltlength >> 32) & 0xFFFFFFFF;
        l_salt4[10] = (tempsaltlength) & 0xFFFFFFFF;

    }

    int k = 0;
    int j = 0;
    bool dataend = false;
    for (int i = 0; i < data4_size; i++) {
        l_data4[i] = 0x00;
        do {
            if (dataend == false) {
                if (k < l_data_length) {
                    l_data4[i] = l_data4[i] << 8;
                    l_data4[i] += l_data[k];
                    k++;
                }
                else if ((k < l_data_length + l_salt_length) && l_salt_length > 0) {
                    l_data4[i] = l_data4[i] << 8;
                    l_data4[i] += l_salt[j++];
                    k++;
                }
                else if (k == l_data_length + l_salt_length) {
                    l_data4[i] = l_data4[i] << 8;
                    l_data4[i] += 0x80;
                    k++;
                }
                else if (k >= l_data_length + l_salt_length) {
                    l_data4[i] = l_data4[i] << 8;
                    k++;
                    if ((k % 4) == 0) {
                        dataend = true;
                    }
                }
            }
        } while ((k % 4) != 0);
    }
    unsigned int blocks = (l_data_length * 8 + l_salt_length * 8 + pad_len) / 512;

    unsigned long tempdatalength = (l_data_length * 8 + l_salt_length * 8);
    l_data4[blocks * 16 - 2] = (tempdatalength >> 32) & 0xFFFFFFFF;
    l_data4[blocks * 16 - 1] = (tempdatalength) & 0xFFFFFFFF;

    return blocks;
}

/*
 * Compute NSEC3 sha1 hash.
 * Supports iterations and input name+salt spanning multiple SHA-1 blocks during initial iteration.
 * Additional iterations must fit into one SHA-1 block, thus salt must be <=35 bytes.
 * l_data must be multiple of 16 four-byte words and blocks must be the multiplier.
 */
void sha1_blocks(__private unsigned int *l_salt, __private unsigned int *l_data, __private unsigned int *digest,
                 unsigned int iterations, unsigned int blocks) {
    unsigned int W[16], temp, A, B, C, D, E;
    unsigned int i, t, v;

    // first iteration over name+salt+padding+length
    v = 0;
    digest[0] = H1;
    digest[1] = H2;
    digest[2] = H3;
    digest[3] = H4;
    digest[4] = H5;
    for (i = 0; i < blocks; ++i) {
        A = digest[0];
        B = digest[1];
        C = digest[2];
        D = digest[3];
        E = digest[4];
        for (t = 0; t < 16; ++t) {
            W[t] = l_data[v++];
        }

#undef R
#define R(t)                                              \
        (                                                         \
            temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^       \
                   W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],        \
            ( W[t & 0x0F] = rotate((int)temp,1) )                 \
        )

#undef P
#define P(a, b, c, d, e, x)                                    \
        {                                                         \
            e += rotate((int)a,5) + F(b,c,d) + K + x; b = rotate((int)b,30);\
        }

#ifdef NVIDIA
#define F(x,y,z)    (z ^ (x & (y ^ z)))
#else
#define F(x, y, z)    bitselect(z, y, x)
#endif
#define K 0x5A827999

        P(A, B, C, D, E, W[0]);
        P(E, A, B, C, D, W[1]);
        P(D, E, A, B, C, W[2]);
        P(C, D, E, A, B, W[3]);
        P(B, C, D, E, A, W[4]);
        P(A, B, C, D, E, W[5]);
        P(E, A, B, C, D, W[6]);
        P(D, E, A, B, C, W[7]);
        P(C, D, E, A, B, W[8]);
        P(B, C, D, E, A, W[9]);
        P(A, B, C, D, E, W[10]);
        P(E, A, B, C, D, W[11]);
        P(D, E, A, B, C, W[12]);
        P(C, D, E, A, B, W[13]);
        P(B, C, D, E, A, W[14]);
        P(A, B, C, D, E, W[15]);
        P(E, A, B, C, D, R(16));
        P(D, E, A, B, C, R(17));
        P(C, D, E, A, B, R(18));
        P(B, C, D, E, A, R(19));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0x6ED9EBA1

        P(A, B, C, D, E, R(20));
        P(E, A, B, C, D, R(21));
        P(D, E, A, B, C, R(22));
        P(C, D, E, A, B, R(23));
        P(B, C, D, E, A, R(24));
        P(A, B, C, D, E, R(25));
        P(E, A, B, C, D, R(26));
        P(D, E, A, B, C, R(27));
        P(C, D, E, A, B, R(28));
        P(B, C, D, E, A, R(29));
        P(A, B, C, D, E, R(30));
        P(E, A, B, C, D, R(31));
        P(D, E, A, B, C, R(32));
        P(C, D, E, A, B, R(33));
        P(B, C, D, E, A, R(34));
        P(A, B, C, D, E, R(35));
        P(E, A, B, C, D, R(36));
        P(D, E, A, B, C, R(37));
        P(C, D, E, A, B, R(38));
        P(B, C, D, E, A, R(39));

#undef K
#undef F

#ifdef NVIDIA
#define F(x,y,z)    ((x & y) | (z & (x | y)))
#else
#define F(x, y, z)    (bitselect(x, y, z) ^ bitselect(x, 0U, y))
#endif
#define K 0x8F1BBCDC

        P(A, B, C, D, E, R(40));
        P(E, A, B, C, D, R(41));
        P(D, E, A, B, C, R(42));
        P(C, D, E, A, B, R(43));
        P(B, C, D, E, A, R(44));
        P(A, B, C, D, E, R(45));
        P(E, A, B, C, D, R(46));
        P(D, E, A, B, C, R(47));
        P(C, D, E, A, B, R(48));
        P(B, C, D, E, A, R(49));
        P(A, B, C, D, E, R(50));
        P(E, A, B, C, D, R(51));
        P(D, E, A, B, C, R(52));
        P(C, D, E, A, B, R(53));
        P(B, C, D, E, A, R(54));
        P(A, B, C, D, E, R(55));
        P(E, A, B, C, D, R(56));
        P(D, E, A, B, C, R(57));
        P(C, D, E, A, B, R(58));
        P(B, C, D, E, A, R(59));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0xCA62C1D6

        P(A, B, C, D, E, R(60));
        P(E, A, B, C, D, R(61));
        P(D, E, A, B, C, R(62));
        P(C, D, E, A, B, R(63));
        P(B, C, D, E, A, R(64));
        P(A, B, C, D, E, R(65));
        P(E, A, B, C, D, R(66));
        P(D, E, A, B, C, R(67));
        P(C, D, E, A, B, R(68));
        P(B, C, D, E, A, R(69));
        P(A, B, C, D, E, R(70));
        P(E, A, B, C, D, R(71));
        P(D, E, A, B, C, R(72));
        P(C, D, E, A, B, R(73));
        P(B, C, D, E, A, R(74));
        P(A, B, C, D, E, R(75));
        P(E, A, B, C, D, R(76));
        P(D, E, A, B, C, R(77));
        P(C, D, E, A, B, R(78));
        P(B, C, D, E, A, R(79));

#undef K
#undef F

        digest[0] += A;
        digest[1] += B;
        digest[2] += C;
        digest[3] += D;
        digest[4] += E;
    }

    // additional iterations over digest+salt+padding+length
    W[0] = digest[0];
    W[1] = digest[1];
    W[2] = digest[2];
    W[3] = digest[3];
    W[4] = digest[4];
    for (i = 1; i <= iterations; i++) {
        A = H1;
        B = H2;
        C = H3;
        D = H4;
        E = H5;

        v = 0;
        for (t = 5; t < 16; ++t) {
            W[t] = l_salt[v++];
        }

#undef R
#define R(t)                                              \
        (                                                         \
            temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^       \
                   W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],        \
            ( W[t & 0x0F] = rotate((int)temp,1) )                 \
        )

#undef P
#define P(a, b, c, d, e, x)                                    \
        {                                                         \
            e += rotate((int)a,5) + F(b,c,d) + K + x; b = rotate((int)b,30);\
        }

#ifdef NVIDIA
#define F(x,y,z)    (z ^ (x & (y ^ z)))
#else
#define F(x, y, z)    bitselect(z, y, x)
#endif
#define K 0x5A827999

        P(A, B, C, D, E, W[0]);
        P(E, A, B, C, D, W[1]);
        P(D, E, A, B, C, W[2]);
        P(C, D, E, A, B, W[3]);
        P(B, C, D, E, A, W[4]);
        P(A, B, C, D, E, W[5]);
        P(E, A, B, C, D, W[6]);
        P(D, E, A, B, C, W[7]);
        P(C, D, E, A, B, W[8]);
        P(B, C, D, E, A, W[9]);
        P(A, B, C, D, E, W[10]);
        P(E, A, B, C, D, W[11]);
        P(D, E, A, B, C, W[12]);
        P(C, D, E, A, B, W[13]);
        P(B, C, D, E, A, W[14]);
        P(A, B, C, D, E, W[15]);
        P(E, A, B, C, D, R(16));
        P(D, E, A, B, C, R(17));
        P(C, D, E, A, B, R(18));
        P(B, C, D, E, A, R(19));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0x6ED9EBA1

        P(A, B, C, D, E, R(20));
        P(E, A, B, C, D, R(21));
        P(D, E, A, B, C, R(22));
        P(C, D, E, A, B, R(23));
        P(B, C, D, E, A, R(24));
        P(A, B, C, D, E, R(25));
        P(E, A, B, C, D, R(26));
        P(D, E, A, B, C, R(27));
        P(C, D, E, A, B, R(28));
        P(B, C, D, E, A, R(29));
        P(A, B, C, D, E, R(30));
        P(E, A, B, C, D, R(31));
        P(D, E, A, B, C, R(32));
        P(C, D, E, A, B, R(33));
        P(B, C, D, E, A, R(34));
        P(A, B, C, D, E, R(35));
        P(E, A, B, C, D, R(36));
        P(D, E, A, B, C, R(37));
        P(C, D, E, A, B, R(38));
        P(B, C, D, E, A, R(39));

#undef K
#undef F

#ifdef NVIDIA
#define F(x,y,z)    ((x & y) | (z & (x | y)))
#else
#define F(x, y, z)    (bitselect(x, y, z) ^ bitselect(x, 0U, y))
#endif
#define K 0x8F1BBCDC

        P(A, B, C, D, E, R(40));
        P(E, A, B, C, D, R(41));
        P(D, E, A, B, C, R(42));
        P(C, D, E, A, B, R(43));
        P(B, C, D, E, A, R(44));
        P(A, B, C, D, E, R(45));
        P(E, A, B, C, D, R(46));
        P(D, E, A, B, C, R(47));
        P(C, D, E, A, B, R(48));
        P(B, C, D, E, A, R(49));
        P(A, B, C, D, E, R(50));
        P(E, A, B, C, D, R(51));
        P(D, E, A, B, C, R(52));
        P(C, D, E, A, B, R(53));
        P(B, C, D, E, A, R(54));
        P(A, B, C, D, E, R(55));
        P(E, A, B, C, D, R(56));
        P(D, E, A, B, C, R(57));
        P(C, D, E, A, B, R(58));
        P(B, C, D, E, A, R(59));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0xCA62C1D6

        P(A, B, C, D, E, R(60));
        P(E, A, B, C, D, R(61));
        P(D, E, A, B, C, R(62));
        P(C, D, E, A, B, R(63));
        P(B, C, D, E, A, R(64));
        P(A, B, C, D, E, R(65));
        P(E, A, B, C, D, R(66));
        P(D, E, A, B, C, R(67));
        P(C, D, E, A, B, R(68));
        P(B, C, D, E, A, R(69));
        P(A, B, C, D, E, R(70));
        P(E, A, B, C, D, R(71));
        P(D, E, A, B, C, R(72));
        P(C, D, E, A, B, R(73));
        P(B, C, D, E, A, R(74));
        P(A, B, C, D, E, R(75));
        P(E, A, B, C, D, R(76));
        P(D, E, A, B, C, R(77));
        P(C, D, E, A, B, R(78));
        P(B, C, D, E, A, R(79));

#undef K
#undef F

        W[0] = A + H1;
        W[1] = B + H2;
        W[2] = C + H3;
        W[3] = D + H4;
        W[4] = E + H5;
    }

    digest[0] = W[0];
    digest[1] = W[1];
    digest[2] = W[2];
    digest[3] = W[3];
    digest[4] = W[4];
}

__kernel void preparedata_sha1_kernel(__global infos_struct *data_info,
                                      __global unsigned char *data,
                                      __global unsigned char *salt,
                                      __global unsigned char *digest)
{
        __private int gid;

        gid = get_global_id(0);


        __private infos_struct infos = data_info[gid];

        __private unsigned int l_data_length = infos.dataLengthGlobal;
        __private unsigned int l_salt_length = infos.saltLengthGlobal;
        __private unsigned int l_iterations = infos.iterations;
        __private unsigned int l_item_domain_length = infos.itemDomainLength;
        __private unsigned int l_item_salt_length = infos.itemSaltLength;


        __private unsigned char domain[64];
        __private unsigned char zalt[64];

        uint domainOffset = gid*l_data_length;
        uint saltOffset = gid*l_salt_length;

        for (int i=0; i<l_item_domain_length;i++) {
            domain[i] = data[domainOffset+i];
        }

        for (int i=0; i<l_item_salt_length;i++) {
            zalt[i] = salt[saltOffset+i];
        }

        // Prep for sha1Blocks
        __private unsigned int l_data4[32];
        __private unsigned int l_salt4[11];
        __private unsigned int l_blocks;

        l_blocks = prepare_data(domain, l_item_domain_length, zalt, l_item_salt_length, l_data4, l_salt4);

        __private unsigned int l_digest[5];

        // Perform SHA1
        sha1_blocks(l_salt4, l_data4, l_digest, l_iterations, l_blocks);

        // Convert to uchars
        __private unsigned char l_hashhex[20];
        post_work(l_digest, l_hashhex);


        for(int t = 0;t < 20; t++) {
            digest[gid * 20 +t] = l_hashhex[t];
        }
}
