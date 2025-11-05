#include "server.h"
#include <thread>

bool Server::Initialize() {
    std::cout << "Server initializing..." << std::endl;
    return serverSocket.Create() && serverSocket.Bind(DEFAULT_PORT) && serverSocket.Listen();
}

void Server::Run() {
    std::cout << "Server running on port " << DEFAULT_PORT << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    while (!stopRequested) {
        if (serverSocket.CanRead(1000)) {
            auto clientSocket = serverSocket.Accept();
            if (clientSocket) {
                std::cout << "Client connected" << std::endl;
                std::thread clientThread(&Server::ClientHandler, this, std::move(clientSocket));
                clientThread.detach(); 
            }
        }
    }
}

void Server::Stop() {
    stopRequested = true;
    serverSocket.Close();
}

void Server::ClientHandler(std::unique_ptr<SocketWrapper> clientSocket) {
    if (!clientSocket) return;
    
    std::cout << "Client handler started" << std::endl;
    
    char buffer[BUFFER_SIZE];
    while (clientSocket->IsConnected()) {
        if (clientSocket->CanRead(1000)) {
            int bytesReceived = clientSocket->Recv(buffer, BUFFER_SIZE - 1);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Received: " << buffer;
                
                std::string response = "Echo: " + std::string(buffer);
                clientSocket->Send(response.c_str(), static_cast<int>(response.length()));
            }
            else if (bytesReceived == 0) {
                break; 
            }
        }
    }
    
    std::cout << "Client disconnected" << std::endl;
}
