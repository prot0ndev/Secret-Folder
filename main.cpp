#include <iostream>
#include <string>
#include <windows.h>
#include <direct.h>
#include <cctype>

const std::string LOCKED_FOLDER_NAME = "Control Panel.{21EC2020-3AEA-1069-A2DD-08002B30309D}";
const std::string PRIVATE_FOLDER_NAME = "Private";
const std::string PASSWORD = "ballsofsigma123";

bool directoryExists(const std::string& dirName_in) {
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;
    return false;
}

void lockFolder() {
    if (MoveFileA(PRIVATE_FOLDER_NAME.c_str(), LOCKED_FOLDER_NAME.c_str())) {
        SetFileAttributesA(LOCKED_FOLDER_NAME.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        std::cout << "Folder locked" << std::endl;
    } else {
        std::cerr << "Error locking folder." << std::endl;
    }
}

void unlockFolder() {
    std::cout << "Enter your passkey to unlock the secure folder: ";
    std::string pass;
    std::getline(std::cin >> std::ws, pass);
    if (pass == PASSWORD) {
        SetFileAttributesA(LOCKED_FOLDER_NAME.c_str(), FILE_ATTRIBUTE_NORMAL);
        if (MoveFileA(LOCKED_FOLDER_NAME.c_str(), PRIVATE_FOLDER_NAME.c_str())) {
            std::cout << "Folder Unlocked successfully" << std::endl;
        } else {
            std::cerr << "Error unlocking folder." << std::endl;
        }
    } else {
        std::cout << "Invalid password" << std::endl;
    }
}

void createPrivateFolder() {
    if (_mkdir(PRIVATE_FOLDER_NAME.c_str()) == 0) {
        std::cout << "Private created successfully" << std::endl;
    } else {
        std::cerr << "Error creating Private folder." << std::endl;
    }
}

void confirmLock() {
    while (true) {
        std::cout << "Are you sure to lock this folder? (Y/N) ";
        std::string input;
        std::getline(std::cin >> std::ws, input);

        if (!input.empty()) {
            char choice = std::toupper(static_cast<unsigned char>(input[0]));
            if (choice == 'Y') {
                lockFolder();
                break;
            } else if (choice == 'N') {
                break;
            }
        }

        std::cout << "Invalid choice." << std::endl;
    }
}

int main() {
    if (directoryExists(LOCKED_FOLDER_NAME)) {
        unlockFolder();
    } else if (directoryExists(PRIVATE_FOLDER_NAME)) {
        confirmLock();
    } else {
        createPrivateFolder();
    }
    return 0;
}
