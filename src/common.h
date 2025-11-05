#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")

#define DEFAULT_PORT "27015"
#define BUFFER_SIZE 4096

enum class AppMode {
    CLIENT,
    SERVER,
    SERVICE
};

struct Config {
    AppMode mode = AppMode::SERVER;
    std::string host = "127.0.0.1";
    std::string port = DEFAULT_PORT;
    bool isService = false;
};

class SocketWrapper;
class Server;
class Client;
class ServiceController;
