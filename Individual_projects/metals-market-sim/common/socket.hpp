#pragma once
// -----------------------------------------------------------------------------
// Thin C++ wrappers over the POSIX (Linux) socket API.
//
// A socket is just a FILE DESCRIPTOR (an int) the kernel gave you. Everything
// is done with system calls on that int:
//
//   TCP server: socket() -> setsockopt(SO_REUSEADDR) -> bind() -> listen() -> accept() ...
//   TCP client: socket() -> connect()
//   UDP:        socket(SOCK_DGRAM) -> bind() (optional) -> sendto()/recvfrom()
//
// Man pages are the real documentation:  man 2 socket, man 7 tcp, man 7 udp,
// man 7 ip, man 2 accept, man 3 getaddrinfo. (In WSL: sudo apt install manpages-dev)
//
// Every syscall can fail. Check the return value, and on error read `errno`
// (std::strerror(errno) gives readable text). Throwing on setup failures is fine.
// -----------------------------------------------------------------------------
#include <netinet/in.h>  // sockaddr_in (Linux-only header: build in WSL/Docker)

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace mms::net {

// RAII owner of a file descriptor: closes it automatically when destroyed.
// Move-only. Two owners of one fd = double close = closing some OTHER
// socket that reused the number. A nasty bug.
//
// This is the core C++ RAII idea: resource lifetime == object lifetime.
class Fd {
public:
    Fd() = default;
    explicit Fd(int fd) : fd_(fd) {}
    ~Fd();

    Fd(const Fd&) = delete;             // no copies
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept;            // moves transfer ownership
    Fd& operator=(Fd&& other) noexcept;

    int get() const { return fd_; }
    bool valid() const { return fd_ >= 0; }
    int release();             // give up ownership without closing; returns the fd
    void reset(int fd = -1);   // close the current fd (if any) and own a new one

private:
    int fd_ = -1;
};

// Listening TCP socket on 0.0.0.0:port (all interfaces).
// 0.0.0.0, NOT 127.0.0.1: in Docker, 127.0.0.1 is only reachable from inside
// the same container!
Fd tcp_listen(uint16_t port, int backlog = 128);

// Blocking connect. Throws on failure.
Fd tcp_connect(const std::string& host, uint16_t port);

// Keep trying (services start in random order under Docker). Throws after the last attempt.
Fd tcp_connect_retry(const std::string& host, uint16_t port, int attempts, std::chrono::milliseconds delay);

// UDP socket bound to 0.0.0.0:port. port 0 = "kernel, pick any free port".
Fd udp_socket(uint16_t bind_port = 0);

void set_nonblocking(int fd);  // fcntl(O_NONBLOCK): recv/send return EAGAIN instead of waiting
void set_nodelay(int fd);      // TCP_NODELAY: disable Nagle's algorithm (see connection.hpp)

// DNS: "engine_xau" -> 172.20.0.5. Docker runs a DNS server at 127.0.0.11 inside
// each container that resolves service names. Try: docker compose exec bots dig engine_xau
std::optional<sockaddr_in> resolve(const std::string& host, uint16_t port);

std::string to_string(const sockaddr_in& addr);  // "172.20.0.5:9200"
std::string peer_name(int fd);                   // getpeername() + to_string

}  // namespace mms::net
