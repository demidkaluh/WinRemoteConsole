#pragma once
#include "../common.h"
#include "../wrappers/socket_wrapper.h"

class Client {
private:
    SocketWrapper clientSocket;
    std::atomic<bool> stopRequested{false};

public:
    bool Connect(const std::string& host, const std::string& port);
    void Run();
    void Stop();
    void ReceiveHandler();
};
