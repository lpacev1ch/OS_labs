#include <windows.h>
#include <iostream>
#include <fstream>

int main() {
    char filename[MAX_PATH];
    std::cout << "Enter binary file name: ";
    std::cin >> filename;

    
    std::ofstream ofs(filename, std::ios::binary);
    char empty[20] = {};
    ofs.write(empty, 20);
    ofs.close();

    
    HANDLE eventSenderReady = CreateEvent(NULL, FALSE, FALSE, "SenderReady");
    HANDLE eventReceiverReady = CreateEvent(NULL, FALSE, TRUE, "ReceiverReady");
    

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    std::string cmd = "Sender.exe " + std::string(filename);
    CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    std::string command;
    while (true) {
        std::cout << "Enter command (read / exit): ";
        std::cin >> command;
        if (command == "read") {
            WaitForSingleObject(eventSenderReady, INFINITE);

            std::ifstream ifs(filename, std::ios::binary);
            char buffer[21] = {};
            ifs.read(buffer, 20);
            ifs.close();
            std::cout << "Received: " << buffer << std::endl;

            SetEvent(eventReceiverReady);
        } else if (command == "exit") {
            break;
        }
    }

    CloseHandle(eventSenderReady);
    CloseHandle(eventReceiverReady);
    return 0;
}
