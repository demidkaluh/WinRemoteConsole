#include "client.h"
#include <thread>

bool Client::Connect(const std::string& host, const std::string& port) {
    std::cout << "Connecting to " << host << ":" << port << std::endl;
    return clientSocket.Create() && clientSocket.Connect(host, port);
}

void Client::Run() {
    std::cout << "Client running. Type 'exit' to quit." << std::endl;
    
    std::thread receiveThread(&Client::ReceiveHandler, this);
    
    std::string input;
    while (!stopRequested && std::getline(std::cin, input)) {
        if (input == "exit") break;
        input += "\n";
        clientSocket.Send(input.c_str(), static_cast<int>(input.length()));
    }

    Stop();
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
}

void Client::Stop() {
    stopRequested = true;
    clientSocket.Close();
}

void Client::ReceiveHandler() {
    while (!stopRequested) {
        if (clientSocket.CanRead(1000)) {
            char buffer[BUFFER_SIZE];
            int bytesReceived = clientSocket.Recv(buffer, BUFFER_SIZE - 1);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << buffer;
                std::cout.flush();
            }
            else if (bytesReceived == 0) {
                std::cout << "\nConnection closed by server" << std::endl;
                break;
            }
        }
    }
}
