#include "common.h"
#include "client/client.h"
#include "server/server.h"
#include "service/service.h"

void PrintUsage() {
    std::cout << "RemoteConsole.exe Usage:" << std::endl;
    std::cout << "  Server mode:          RemoteConsole.exe -s [port]" << std::endl;
    std::cout << "  Client mode:          RemoteConsole.exe -c [host] [port]" << std::endl;
    std::cout << "  Service mode:         RemoteConsole.exe -service" << std::endl;
    std::cout << "  Install service:      RemoteConsole.exe -install" << std::endl;
    std::cout << "  Uninstall service:    RemoteConsole.exe -uninstall" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  RemoteConsole.exe -s 27015           # Start server on port 27015" << std::endl;
    std::cout << "  RemoteConsole.exe -c 192.168.1.100   # Connect to server at 192.168.1.100:27015" << std::endl;
    std::cout << "  RemoteConsole.exe -service           # Run as Windows service" << std::endl;
}

Config ParseArguments(int argc, char* argv[]) {
    Config config;
    
    if (argc < 2) {
        PrintUsage();
        exit(1);
    }

    std::string mode = argv[1];
    
    if (mode == "-c") {
        config.mode = AppMode::CLIENT;
        if (argc >= 3) config.host = argv[2];
        if (argc >= 4) config.port = argv[3];
    }
    else if (mode == "-s") {
        config.mode = AppMode::SERVER;
        if (argc >= 3) config.port = argv[2];
    }
    else if (mode == "-service") {
        config.mode = AppMode::SERVICE;
    }
    else if (mode == "-install") {
        ServiceController::InstallService();
        exit(0);
    }
    else if (mode == "-uninstall") {
        ServiceController::UninstallService();
        exit(0);
    }
    else {
        PrintUsage();
        exit(1);
    }

    return config;
}

int main(int argc, char* argv[]) {
    Config config = ParseArguments(argc, argv);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    int result = 0;
    
    try {
        if (config.mode == AppMode::SERVER) {
            Server server;
            if (server.Initialize()) {
                std::cout << "Server started successfully" << std::endl;
                server.Run();
            } else {
                std::cerr << "Server initialization failed" << std::endl;
                result = 1;
            }
        }
        else if (config.mode == AppMode::CLIENT) {
            Client client;
            if (client.Connect(config.host, config.port)) {
                std::cout << "Connected successfully" << std::endl;
                client.Run();
            } else {
                std::cerr << "Connection failed" << std::endl;
                result = 1;
            }
        }
        else if (config.mode == AppMode::SERVICE) {
            ServiceController::Run();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        result = 1;
    }

    WSACleanup();
    return result;
}
