#include "UDPReceiver.h"
#include <sys/socket.h>
UDPSocket::UDPSocket(const int port) {
    int rc;
    struct sockaddr_in servAddr;

    const int y = 1;

    socketId = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketId < 0) {
        std::cout << "Could not open UDP socket: " << strerror(errno) << std::endl;
        // Todo throw ?
    } else {
        std::cout << "socket open" << std::endl;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
    setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    setsockopt(socketId, SOL_SOCKET, SO_REUSEPORT, &y, sizeof(int));

   // int size = 2147483647;
    int size = 100 * 1024 * 1024;
    setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, &size, (socklen_t)sizeof(int));
    setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, &size, (socklen_t)sizeof(int));


    /* struct timeval tv;
     tv.tv_sec = 0;
     tv.tv_usec = 100000;
     if (setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
         perror("Error");
     }*/

    rc = bind(socketId, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if (rc < 0) {
        std::cout << "Could not bind to port " << port << strerror(errno) << std::endl;
    } else {
        std::cout << "Bound successfully to port " << port << std::endl;
        // Todo throw
    }

    std::cout << "Listening on UDP port" << port << std::endl;
}

UDPSocket::~UDPSocket() {
    int closeResult = close(this->socketId);
    std::cout << "Closed socket, result is " << closeResult << std::endl;
}

void UDPSocket::stop() const {
    int shutdownResult = shutdown(this->socketId, SHUT_RDWR);
    std::cout << "Shutdown on socket called, result is " << shutdownResult << std::endl;
}

ssize_t UDPSocket::receive(char *buffer, size_t bufferSize,
                           struct sockaddr_in *clientAddress, socklen_t *clientLength) const {
    //FIXME int <-> ssize_t
    return recvfrom(socketId, buffer, bufferSize, 0,
                    (sockaddr *) clientAddress, clientLength);
}

ssize_t UDPSocket::sendTo(char *buffer, size_t bufferSize, struct sockaddr_in *targetAddress,
                          socklen_t targetAddressLength) {
    //FIXME int <-> ssize_t
    return sendto(socketId, buffer, bufferSize, /*MSG_DONTWAIT*/ 0, (sockaddr *) targetAddress, targetAddressLength);
}

int UDPSocket::sendMmsg(struct mmsghdr *msgvec, unsigned int vlen, unsigned int flags) {
    return sendmmsg(socketId, msgvec, vlen, flags);
}