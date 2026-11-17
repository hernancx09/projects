#include "common/socket.hpp"

#include <arpa/inet.h>    // inet_ntop, htons
#include <fcntl.h>        // fcntl, O_NONBLOCK
#include <netdb.h>        // getaddrinfo
#include <netinet/tcp.h>  // TCP_NODELAY
#include <sys/socket.h>
#include <unistd.h>       // close

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <thread>

#include "common/log.hpp"
#include "common/todo.hpp"

namespace mms::net {

// ======================================================================= Fd
// TODO(P1): implement these four. They're short but they MATTER.
Fd::~Fd() {
    // TODO(P1): if (fd_ >= 0) ::close(fd_);
    // (Destructors must not throw, which is why this doesn't call not_implemented().
    //  Until you implement it, every socket LEAKS.)
}

Fd::Fd(Fd&& other) noexcept : fd_(other.fd_) {
    // TODO(P1): `other` must forget the fd, or both objects close it: other.fd_ = -1;
}

Fd& Fd::operator=(Fd&& other) noexcept {
    // TODO(P1): if (this != &other) { reset(other.release()); }
    return *this;
}

int Fd::release() {
    // TODO(P1): int fd = fd_; fd_ = -1; return fd;
    return fd_;
}

void Fd::reset(int fd) {
    // TODO(P1): close the old fd if valid, then fd_ = fd
    fd_ = fd;
}

// ================================================================== helpers
Fd tcp_listen(uint16_t port, int backlog) {
    // TODO(P1):
    //   Fd fd(::socket(AF_INET, SOCK_STREAM, 0));   check fd.valid()
    //   int one = 1; setsockopt(fd.get(), SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    //     ^ without this, restarting a server fails with "Address already in use" for
    //       ~60s while old connections sit in TIME_WAIT. (Look up TIME_WAIT. Good to know.)
    //   sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(port);
    //   addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //     ^ htons/htonl = "host to network short/long" = convert to big-endian.
    //       Same idea as your ByteWriter, done by the C library.
    //   bind(fd.get(), reinterpret_cast<sockaddr*>(&addr), sizeof addr)
    //   listen(fd.get(), backlog)
    //     ^ backlog = how many finished handshakes can wait in the kernel for accept()
    //   on any failure: throw std::runtime_error(std::string("bind: ") + std::strerror(errno))
    not_implemented();
}

Fd tcp_connect(const std::string& host, uint16_t port) {
    // TODO(P1): auto addr = resolve(host, port); socket(); ::connect(...)
    //   connect() does the 3-way handshake (SYN, SYN-ACK, ACK). Capture it in
    //   tcpdump/Wireshark and find all three packets!
    not_implemented();
}

Fd tcp_connect_retry(const std::string& host, uint16_t port, int attempts, std::chrono::milliseconds delay) {
    // TODO(P2): loop: try { return tcp_connect(host, port); } catch (...) { log::warn(...); std::this_thread::sleep_for(delay); }
    //   rethrow / throw after the last attempt.
    // TODO(STRETCH): exponential backoff with jitter (delay *= 2, plus a random bit).
    //   Why jitter? So 50 bots that lost the connection at the same moment
    //   don't all retry at the same moment ("thundering herd").
    not_implemented();
}

Fd udp_socket(uint16_t bind_port) {
    // TODO(P4): socket(AF_INET, SOCK_DGRAM, 0), bind to INADDR_ANY:bind_port (even port 0, so
    //   the kernel assigns a port NOW and getsockname() can tell you which one).
    // TODO(STRETCH): raise SO_RCVBUF (setsockopt) and see if it reduces drops under load.
    //   When the receive buffer is full, the kernel silently DROPS incoming datagrams.
    not_implemented();
}

void set_nonblocking(int fd) {
    // TODO(P2): int flags = fcntl(fd, F_GETFL, 0); fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    not_implemented();
}

void set_nodelay(int fd) {
    // TODO(P2): int one = 1; setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    not_implemented();
}

std::optional<sockaddr_in> resolve(const std::string& host, uint16_t port) {
    // TODO(P1):
    //   addrinfo hints{}; hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
    //   addrinfo* res = nullptr;
    //   if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) return std::nullopt;
    //   copy res->ai_addr into a sockaddr_in, set sin_port = htons(port)
    //   freeaddrinfo(res);   <- C API: YOU must free it. (TODO(STRETCH): wrap in a unique_ptr with a custom deleter)
    // Note: getaddrinfo BLOCKS. Fine at startup, bad inside the event loop.
    not_implemented();
}

std::string to_string(const sockaddr_in& addr) {
    // TODO(P1): inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof buf) + ":" + std::to_string(ntohs(addr.sin_port))
    not_implemented();
}

std::string peer_name(int fd) {
    // TODO(P1): sockaddr_in a{}; socklen_t len = sizeof a; getpeername(fd, (sockaddr*)&a, &len); return to_string(a);
    not_implemented();
}

}  // namespace mms::net
