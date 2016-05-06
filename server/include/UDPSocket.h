#ifndef GPUDNS_UDPSOCKET_H
#define GPUDNS_UDPSOCKET_H

#define BUF 255

#include <unistd.h>

class UDPSocket {
public:
    UDPSocket(int const port);

    UDPSocket(const UDPSocket &) = delete;

    ~UDPSocket();

    void stop() const;

    ssize_t receive(char *buffer, size_t bufferSize, struct sockaddr_in *clientAddress,
                socklen_t *clientLength) const;

    ssize_t sendTo(char *buffer, size_t bufferSize, struct sockaddr_in *targetAddress, socklen_t targetAddressLength);

    int sendMmsg(struct mmsghdr *msgvec, unsigned int vlen, unsigned int flags);

private:
    int socketId;
};

#endif