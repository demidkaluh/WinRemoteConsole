#pragma once
#include "../common.h"
#include "../wrappers/socket_wrapper.h"

class Server {
private:
    SocketWrapper serverSocket;
    std::atomic<bool> stopRequested{false};

public:
    bool Initialize();
    void Run();
    void Stop();
    void ClientHandler(std::unique_ptr<SocketWrapper> clientSocket);
};
