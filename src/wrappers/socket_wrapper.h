#pragma once
#include "../common.h"

class SocketWrapper {
private:
    SOCKET socket_ = INVALID_SOCKET;
    bool connected_ = false;
    bool blocking_ = true;

public:
    SocketWrapper() = default;
    explicit SocketWrapper(SOCKET socket);
    ~SocketWrapper();
    
    SocketWrapper(const SocketWrapper&) = delete;
    SocketWrapper& operator=(const SocketWrapper&) = delete;
    
    SocketWrapper(SocketWrapper&& other) noexcept;
    SocketWrapper& operator=(SocketWrapper&& other) noexcept;
    
    bool Create(int af = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
    bool Connect(const std::string& host, const std::string& port);
    bool Bind(const std::string& port);
    bool Listen(int backlog = SOMAXCONN);
    std::unique_ptr<SocketWrapper> Accept();
    
    int Send(const char* buffer, int length, int flags = 0);
    int Recv(char* buffer, int length, int flags = 0);
    bool SendAll(const char* buffer, int length);
    
    bool SetBlocking(bool blocking);
    bool WaitForData(int timeout_ms = 1000);
    bool CanRead(int timeout_ms = 0);
    bool CanWrite(int timeout_ms = 0);
    
    void Close();
    void Shutdown(int how = SD_BOTH);
    
    bool IsValid() const { return socket_ != INVALID_SOCKET; }
    bool IsConnected() const { return connected_; }
    SOCKET GetSocket() const { return socket_; }
    
    static bool InitializeWinsock();
    static void CleanupWinsock();
    static std::string GetLastErrorString();
    
private:
    bool SetSocketOption(int level, int optname, const void* optval, int optlen);
};
