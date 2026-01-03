#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <random>
#include "plusaes.hpp"
#include "crypto/hmac.h"
#include "crypto/sha256.h"


namespace fs = std::filesystem;

const std::string LOCKED_VAULT_NAME = "Control Panel.{21EC2020-3AEA-1069-A2DD-08002B30309D}";
const std::string PRIVATE_FOLDER_NAME = "Private";
const unsigned int PBKDF2_ITERATIONS = 310000;

// Helper to convert hex string to bytes
std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = (unsigned char) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

// Raw byte HMAC-SHA256 helper
std::vector<unsigned char> hmac_sha256(const std::vector<unsigned char>& data, const std::string& key) {
    SHA256 sha256;
    std::string hash_str = hmac<SHA256>(data.data(), data.size(), key.c_str(), key.size());
    return hex_to_bytes(hash_str);
}

// Correct PBKDF2-HMAC-SHA256 implementation
std::vector<unsigned char> pbkdf2(const std::string& password, const std::vector<unsigned char>& salt, int iterations, int dkLen) {
    std::vector<unsigned char> dk;
    dk.reserve(dkLen);
    int hLen = SHA256::HashBytes;
    int l = (dkLen + hLen - 1) / hLen;

    for (int i = 1; i <= l; i++) {
        std::vector<unsigned char> salt_and_i = salt;
        salt_and_i.push_back((i >> 24) & 0xFF);
        salt_and_i.push_back((i >> 16) & 0xFF);
        salt_and_i.push_back((i >> 8) & 0xFF);
        salt_and_i.push_back(i & 0xFF);

        std::vector<unsigned char> U = hmac_sha256(salt_and_i, password);

        std::vector<unsigned char> F = U;
        for (int j = 1; j < iterations; j++) {
            U = hmac_sha256(U, password);
            for (int k = 0; k < hLen; k++) {
                F[k] ^= U[k];
            }
        }
        dk.insert(dk.end(), F.begin(), F.end());
    }
    dk.resize(dkLen);
    return dk;
}

// Generates a random byte vector
std::vector<unsigned char> random_bytes(size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::vector<unsigned char> bytes(size);
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = static_cast<unsigned char>(dis(gen));
    }
    return bytes;
}

bool serialize_folder(std::vector<unsigned char>& buffer) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(PRIVATE_FOLDER_NAME)) {
            std::string path_str = fs::relative(entry.path(), PRIVATE_FOLDER_NAME).string();
            std::replace(path_str.begin(), path_str.end(), '\\', '/'); // Normalize path separators
            if (path_str == ".") continue;

            buffer.insert(buffer.end(), path_str.begin(), path_str.end());
            buffer.push_back('\0');

            if (fs::is_directory(entry.path())) {
                uint64_t size = -1; // Special marker for directory
                const unsigned char* size_bytes = reinterpret_cast<const unsigned char*>(&size);
                buffer.insert(buffer.end(), size_bytes, size_bytes + sizeof(size));
            } else {
                uint64_t size = fs::file_size(entry.path());
                const unsigned char* size_bytes = reinterpret_cast<const unsigned char*>(&size);
                buffer.insert(buffer.end(), size_bytes, size_bytes + sizeof(size));

                std::ifstream file(entry.path(), std::ios::binary);
                if (!file) continue;
                buffer.insert(buffer.end(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error serializing folder: " << e.what() << std::endl;
        return false;
    }
}

bool deserialize_folder(const std::vector<unsigned char>& buffer) {
    try {
        fs::create_directory(PRIVATE_FOLDER_NAME);
        size_t pos = 0;
        while (pos < buffer.size()) {
            auto path_end_it = std::find(buffer.begin() + pos, buffer.end(), '\0');
            std::string path_str(buffer.begin() + pos, path_end_it);
            pos = std::distance(buffer.begin(), path_end_it) + 1;

            fs::path full_path = fs::path(PRIVATE_FOLDER_NAME) / path_str;

            uint64_t size;
            memcpy(&size, buffer.data() + pos, sizeof(size));
            pos += sizeof(size);

            if (size == (uint64_t)-1) { // Directory marker
                fs::create_directories(full_path);
            } else {
                if (!full_path.parent_path().empty()) {
                    fs::create_directories(full_path.parent_path());
                }
                std::ofstream file(full_path, std::ios::binary);
                if (!file) return false;
                file.write(reinterpret_cast<const char*>(buffer.data() + pos), size);
                pos += size;
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deserializing folder: " << e.what() << std::endl;
        return false;
    }
}


bool lock_folder(const std::string& password) {
    std::vector<unsigned char> buffer;
    if (!serialize_folder(buffer)) return false;

    auto salt = random_bytes(16);
    auto key = pbkdf2(password, salt, PBKDF2_ITERATIONS, 32);
    auto iv = random_bytes(16);

    const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(buffer.size());
    std::vector<unsigned char> encrypted(encrypted_size);

    plusaes::encrypt_cbc(buffer.data(), buffer.size(), &key[0], key.size(), (const unsigned char (*)[16])iv.data(), &encrypted[0], encrypted.size(), true);

    std::ofstream vault(LOCKED_VAULT_NAME, std::ios::binary);
    vault.write(reinterpret_cast<const char*>(salt.data()), salt.size());
    vault.write(reinterpret_cast<const char*>(iv.data()), iv.size());
    vault.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
    vault.close();

    SetFileAttributesA(LOCKED_VAULT_NAME.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    fs::remove_all(PRIVATE_FOLDER_NAME);

    std::cout << "Folder locked successfully." << std::endl;
    return true;
}

bool unlock_folder(const std::string& password) {
    std::ifstream vault(LOCKED_VAULT_NAME, std::ios::binary);
    if (!vault) return false;

    std::vector<unsigned char> salt(16);
    vault.read(reinterpret_cast<char*>(salt.data()), salt.size());

    std::vector<unsigned char> iv(16);
    vault.read(reinterpret_cast<char*>(iv.data()), iv.size());

    std::vector<unsigned char> encrypted((std::istreambuf_iterator<char>(vault)), std::istreambuf_iterator<char>());
    vault.close();

    auto key = pbkdf2(password, salt, PBKDF2_ITERATIONS, 32);

    std::vector<unsigned char> decrypted(encrypted.size());
    unsigned long padded_size = 0;

    plusaes::Error result = plusaes::decrypt_cbc(encrypted.data(), encrypted.size(), &key[0], key.size(), (const unsigned char (*)[16])iv.data(), &decrypted[0], decrypted.size(), &padded_size);

    if (result != plusaes::kErrorOk) {
        return false;
    }

    std::vector<unsigned char> buffer(decrypted.begin(), decrypted.begin() + encrypted.size() - padded_size);
    deserialize_folder(buffer);

    SetFileAttributesA(LOCKED_VAULT_NAME.c_str(), FILE_ATTRIBUTE_NORMAL);
    fs::remove(LOCKED_VAULT_NAME);

    std::cout << "Folder unlocked successfully." << std::endl;
    return true;
}

int main() {
    bool private_folder_exists = fs::exists(PRIVATE_FOLDER_NAME);
    bool vault_exists = fs::exists(LOCKED_VAULT_NAME);

    if (!private_folder_exists && !vault_exists) {
        std::cout << "No secure folder found. Do you want to create one? (Y/N): ";
        std::string choice_str;
        std::getline(std::cin, choice_str);
        if (!choice_str.empty() && (choice_str[0] == 'y' || choice_str[0] == 'Y')) {
            fs::create_directory(PRIVATE_FOLDER_NAME);
            std::cout << "Folder 'Private' created. Place your files inside and run this program again to lock it." << std::endl;
        }
    } else if (private_folder_exists) {
        std::cout << "Enter a password to lock the folder: ";
        std::string password;
        std::getline(std::cin >> std::ws, password);
        if (!lock_folder(password)) {
             std::cerr << "Failed to lock the folder." << std::endl;
        }
    } else if (vault_exists) {
        std::cout << "Enter password to unlock: ";
        std::string password;
        std::getline(std::cin >> std::ws, password);
        if(!unlock_folder(password)) {
            std::cout << "Invalid password or corrupted vault." << std::endl;
        }
    }

    return 0;
}
