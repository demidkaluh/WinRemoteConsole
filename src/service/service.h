#pragma once
#include "../common.h"
#include "../wrappers/socket_wrapper.h"

class ServiceController {
private:
    static ServiceController* instance;
    SERVICE_STATUS_HANDLE hStatusHandle;
    SERVICE_STATUS status;
    std::atomic<bool> stopRequested;
    SocketWrapper serverSocket;

    static void WINAPI ServiceHandler(DWORD dwControl);
    static void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);

    void UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode = NO_ERROR, DWORD dwWaitHint = 0);
    bool InitializeService();
    void RunService();
    void StopService();

public:
    ServiceController();
    ~ServiceController();

    static bool InstallService();
    static bool UninstallService();
    static void Run();

    void ClientHandler(std::unique_ptr<SocketWrapper> clientSocket);
};
