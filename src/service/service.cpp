#include "service.h"
#include <thread>

ServiceController* ServiceController::instance = nullptr;

ServiceController::ServiceController() : stopRequested(false), hStatusHandle(0) {
    instance = this;
    
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode = NO_ERROR;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
}

ServiceController::~ServiceController() {
    StopService();
}

void ServiceController::UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
    status.dwCurrentState = dwCurrentState;
    status.dwWin32ExitCode = dwWin32ExitCode;
    status.dwWaitHint = dwWaitHint;
    
    if (hStatusHandle) {
        ::SetServiceStatus(hStatusHandle, &status);  
    }
}

bool ServiceController::InitializeService() {
    hStatusHandle = RegisterServiceCtrlHandlerW(L"RemoteConsoleService", ServiceHandler);
    if (!hStatusHandle) {
        return false;
    }
    
    UpdateServiceStatus(SERVICE_START_PENDING);
    return true;
}

void ServiceController::RunService() {
    UpdateServiceStatus(SERVICE_RUNNING);
    
    std::cout << "Remote Console Service is running..." << std::endl;
    
    if (serverSocket.Create() && serverSocket.Bind(DEFAULT_PORT) && serverSocket.Listen()) {
        std::cout << "Service listening on port " << DEFAULT_PORT << std::endl;
        
        while (!stopRequested) {
            if (serverSocket.CanRead(1000)) {
                auto clientSocket = serverSocket.Accept();
                if (clientSocket) {
                    std::thread clientThread(&ServiceController::ClientHandler, this, std::move(clientSocket));
                    clientThread.detach();
                }
            }
        }
        
        serverSocket.Close();
    }
    
    UpdateServiceStatus(SERVICE_STOPPED);
}

void ServiceController::StopService() {
    stopRequested = true;
}

void WINAPI ServiceController::ServiceHandler(DWORD dwControl) {
    switch (dwControl) {
        case SERVICE_CONTROL_STOP:
            if (instance) {
                instance->UpdateServiceStatus(SERVICE_STOP_PENDING);
                instance->StopService();
            }
            break;
        default:
            break;
    }
}

void WINAPI ServiceController::ServiceMain(DWORD argc, LPWSTR* argv) {
    if (instance && instance->InitializeService()) {
        instance->RunService();
    }
}

bool ServiceController::InstallService() {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        std::cout << "Failed to open Service Control Manager" << std::endl;
        return false;
    }

    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, MAX_PATH)) {
        std::cout << "Failed to get module filename" << std::endl;
        CloseServiceHandle(hSCManager);
        return false;
    }

    SC_HANDLE hService = CreateServiceW(
        hSCManager,
        L"RemoteConsoleService",
        L"Remote Console Service",
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        path,
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (!hService) {
        DWORD error = GetLastError();
        std::cout << "Failed to create service. Error: " << error << std::endl;
        CloseServiceHandle(hSCManager);
        return false;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    
    std::cout << "Service installed successfully" << std::endl;
    return true;
}

bool ServiceController::UninstallService() {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        std::cout << "Failed to open Service Control Manager" << std::endl;
        return false;
    }

    SC_HANDLE hService = OpenServiceW(hSCManager, L"RemoteConsoleService", DELETE);
    if (!hService) {
        std::cout << "Service not found or access denied" << std::endl;
        CloseServiceHandle(hSCManager);
        return false;
    }

    bool success = DeleteService(hService) != 0;
    
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    
    if (success) {
        std::cout << "Service uninstalled successfully" << std::endl;
    } else {
        std::cout << "Failed to uninstall service" << std::endl;
    }
    
    return success;
}

void ServiceController::Run() {
    SERVICE_TABLE_ENTRYW serviceTable[] = {
        { (LPWSTR)L"RemoteConsoleService", ServiceMain },
        { nullptr, nullptr }
    };
    
    if (!StartServiceCtrlDispatcherW(serviceTable)) {
        std::cout << "Failed to start service dispatcher. Are you running as service?" << std::endl;
        
        std::cout << "Running in console mode instead..." << std::endl;
        ServiceController controller;
        if (controller.InitializeService()) {
            controller.RunService();
        }
    }
}

void ServiceController::ClientHandler(std::unique_ptr<SocketWrapper> clientSocket) {
    if (!clientSocket) return;
    
    char buffer[BUFFER_SIZE];
    while (clientSocket->IsConnected() && !stopRequested) {
        if (clientSocket->CanRead(1000)) {
            int bytesReceived = clientSocket->Recv(buffer, BUFFER_SIZE - 1);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                
                clientSocket->Send(buffer, bytesReceived);
            }
            else if (bytesReceived == 0) {
                break; 
            }
        }
    }
}
