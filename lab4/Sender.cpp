#include <windows.h>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "File name required\n";
        return 1;
    }

    const char* filename = argv[1];

    HANDLE eventSenderReady = OpenEvent(EVENT_MODIFY_STATE, FALSE, "SenderReady");
    HANDLE eventReceiverReady = OpenEvent(SYNCHRONIZE, FALSE, "ReceiverReady");

    std::string command;
    while (true) {
        std::cout << "Enter command (send / exit): ";
        std::cin >> command;
        if (command == "send") {
            WaitForSingleObject(eventReceiverReady, INFINITE);

            std::cout << "Enter message (<20 chars): ";
            std::string msg;
            std::cin >> msg;
            msg.resize(20, '\0');

            std::ofstream ofs(filename, std::ios::binary);
            ofs.write(msg.c_str(), 20);
            ofs.close();

            SetEvent(eventSenderReady);
        } else if (command == "exit") {
            break;
        }
    }

    return 0;
}
