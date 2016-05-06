#ifndef GPUDNS_UDPPACKET_H
#define GPUDNS_UDPPACKET_H


struct DNS_HEADER {
    //byte 1-2
    unsigned short id; // identification number

    //byte 3
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag

    //byte 4
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available

    //byte 5
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION {
    unsigned short qtype;
    unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA {
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD {
    unsigned char *name;
    struct R_DATA *resource; // fixed fields
    unsigned char *rdata;
};

//Structure of a Query
typedef struct {
    unsigned char *name;
    struct QUESTION *ques;
} QUERY;


typedef struct {
    unsigned char extendedRcode;
    unsigned char version;
    unsigned char z :7;
    unsigned char do_flag :1;
    unsigned char z_remaining;
} EDNS_TTL;

#endif //GPUDNS_UDPPACKET_H
