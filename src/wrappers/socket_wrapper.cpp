#include "socket_wrapper.h"

SocketWrapper::SocketWrapper(SOCKET socket) 
    : socket_(socket), connected_(socket != INVALID_SOCKET) {
}

SocketWrapper::~SocketWrapper() {
    Close();
}

SocketWrapper::SocketWrapper(SocketWrapper&& other) noexcept 
    : socket_(other.socket_), connected_(other.connected_), blocking_(other.blocking_) {
    other.socket_ = INVALID_SOCKET;
    other.connected_ = false;
}

SocketWrapper& SocketWrapper::operator=(SocketWrapper&& other) noexcept {
    if (this != &other) {
        Close();
        socket_ = other.socket_;
        connected_ = other.connected_;
        blocking_ = other.blocking_;
        other.socket_ = INVALID_SOCKET;
        other.connected_ = false;
    }
    return *this;
}

bool SocketWrapper::Create(int af, int type, int protocol) {
    if (socket_ != INVALID_SOCKET) {
        Close();
    }
    
    socket_ = ::socket(af, type, protocol);
    if (socket_ == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << GetLastErrorString() << std::endl;
        return false;
    }
    
    int reuse = 1;
    if (!SetSocketOption(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
        std::cerr << "Failed to set SO_REUSEADDR: " << GetLastErrorString() << std::endl;
    }
    
    return true;
}

bool SocketWrapper::Connect(const std::string& host, const std::string& port) {
    if (!IsValid()) {
        if (!Create()) {
            return false;
        }
    }
    
    addrinfo* result = nullptr;
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0) {
        std::cerr << "getaddrinfo failed: " << GetLastErrorString() << std::endl;
        return false;
    }

    bool connected = false;
    for (addrinfo* ptr = result; ptr != nullptr && !connected; ptr = ptr->ai_next) {
        if (::connect(socket_, ptr->ai_addr, (int)ptr->ai_addrlen) == 0) {
            connected = true;
            connected_ = true;
        }
    }

    freeaddrinfo(result);
    
    if (!connected) {
        std::cerr << "Connect failed: " << GetLastErrorString() << std::endl;
    }
    
    return connected;
}

bool SocketWrapper::Bind(const std::string& port) {
    if (!IsValid()) {
        if (!Create()) {
            return false;
        }
    }
    
    addrinfo* result = nullptr;
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(nullptr, port.c_str(), &hints, &result) != 0) {
        std::cerr << "getaddrinfo failed: " << GetLastErrorString() << std::endl;
        return false;
    }

    bool bound = false;
    for (addrinfo* ptr = result; ptr != nullptr && !bound; ptr = ptr->ai_next) {
        if (::bind(socket_, ptr->ai_addr, (int)ptr->ai_addrlen) == 0) {
            bound = true;
        }
    }

    freeaddrinfo(result);
    
    if (!bound) {
        std::cerr << "Bind failed: " << GetLastErrorString() << std::endl;
    }
    
    return bound;
}

bool SocketWrapper::Listen(int backlog) {
    if (!IsValid()) {
        return false;
    }
    
    if (::listen(socket_, backlog) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << GetLastErrorString() << std::endl;
        return false;
    }
    
    return true;
}

std::unique_ptr<SocketWrapper> SocketWrapper::Accept() {
    if (!IsValid()) {
        return nullptr;
    }
    
    SOCKET clientSocket = ::accept(socket_, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            std::cerr << "Accept failed: " << GetLastErrorString() << std::endl;
        }
        return nullptr;
    }
    
    return std::make_unique<SocketWrapper>(clientSocket);
}

int SocketWrapper::Send(const char* buffer, int length, int flags) {
    if (!IsValid() || !connected_) {
        return SOCKET_ERROR;
    }
    
    int result = ::send(socket_, buffer, length, flags);
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            return 0; 
        }
        connected_ = false;
    }
    
    return result;
}

int SocketWrapper::Recv(char* buffer, int length, int flags) {
    if (!IsValid() || !connected_) {
        return SOCKET_ERROR;
    }
    
    int result = ::recv(socket_, buffer, length, flags);
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            return 0; 
        }
        connected_ = false;
    } else if (result == 0) {
        connected_ = false; 
    }
    
    return result;
}

bool SocketWrapper::SendAll(const char* buffer, int length) {
    int totalSent = 0;
    while (totalSent < length) {
        int sent = Send(buffer + totalSent, length - totalSent);
        if (sent == SOCKET_ERROR) {
            return false;
        }
        if (sent == 0) {
            if (!CanWrite(1000)) {
                return false;
            }
            continue;
        }
        totalSent += sent;
    }
    return true;
}

bool SocketWrapper::SetBlocking(bool blocking) {
    if (!IsValid()) {
        return false;
    }
    
    u_long mode = blocking ? 0 : 1;
    if (ioctlsocket(socket_, FIONBIO, &mode) == SOCKET_ERROR) {
        std::cerr << "SetBlocking failed: " << GetLastErrorString() << std::endl;
        return false;
    }
    
    blocking_ = blocking;
    return true;
}

bool SocketWrapper::WaitForData(int timeout_ms) {
    return CanRead(timeout_ms);
}

bool SocketWrapper::CanRead(int timeout_ms) {
    if (!IsValid()) {
        return false;
    }
    
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(socket_, &readSet);
    
    timeval timeout{ timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    
    int result = select(0, &readSet, nullptr, nullptr, &timeout);
    if (result == SOCKET_ERROR) {
        std::cerr << "Select failed: " << GetLastErrorString() << std::endl;
        return false;
    }
    
    return result > 0 && FD_ISSET(socket_, &readSet);
}

bool SocketWrapper::CanWrite(int timeout_ms) {
    if (!IsValid()) {
        return false;
    }
    
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(socket_, &writeSet);
    
    timeval timeout{ timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    
    int result = select(0, nullptr, &writeSet, nullptr, &timeout);
    if (result == SOCKET_ERROR) {
        std::cerr << "Select failed: " << GetLastErrorString() << std::endl;
        return false;
    }
    
    return result > 0 && FD_ISSET(socket_, &writeSet);
}

void SocketWrapper::Close() {
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
        connected_ = false;
    }
}

void SocketWrapper::Shutdown(int how) {
    if (IsValid() && connected_) {
        ::shutdown(socket_, how);
    }
}

bool SocketWrapper::SetSocketOption(int level, int optname, const void* optval, int optlen) {
    if (!IsValid()) {
        return false;
    }
    
    return ::setsockopt(socket_, level, optname, (const char*)optval, optlen) != SOCKET_ERROR;
}

bool SocketWrapper::InitializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    return true;
}

void SocketWrapper::CleanupWinsock() {
    WSACleanup();
}

std::string SocketWrapper::GetLastErrorString() {
    int error = WSAGetLastError();
    char buffer[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buffer, sizeof(buffer), nullptr);
    return std::string(buffer);
}
