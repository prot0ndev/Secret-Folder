# Secret Folder

This project provides a C++ program that creates a secure, password-protected folder. It improves upon the original batch script by using strong AES-256 encryption to create a truly secure "vault" for your files.

## How It Works

The program operates in three main modes:

1.  **First Run (Creation):** If no secure folder or vault exists, the program will ask if you want to create one. If you agree, it will create a folder named `Private`. You can then place your sensitive files and folders inside this `Private` directory.

2.  **Locking:** Once you have placed your files in the `Private` folder, run the program again. It will ask for a password, which it will use to encrypt the folder's contents. The program then:
    *   Generates a random salt and derives a strong encryption key from your password using **PBKDF2**.
    *   Generates a random **Initialization Vector (IV)**.
    *   Serializes the entire `Private` folder into a single data buffer.
    *   Encrypts this buffer using **AES-256-CBC**.
    *   Saves the salt, IV, and encrypted data to a file named `Control Panel.{21EC2020-3AEA-1069-A2DD-08002B30309D}`.
    *   Hides this "vault" file using system attributes.
    *   Deletes the original `Private` folder.

3.  **Unlocking:** When you run the program and the hidden vault file is present, it will ask for the password. If the password is correct, the program will:
    *   Read the salt, IV, and encrypted data from the vault file.
    *   Re-derive the encryption key using the password and salt.
    *   Decrypt the data.
    *   Reconstruct the original `Private` folder and all its contents.
    *   Delete the vault file, leaving the folder unlocked.

## Building the Executable

To compile this program, you will need a C++ compiler that supports C++17 (for the `<filesystem>` library). MinGW-w64 on Windows is a good choice.

1.  **Install a C++ compiler:** If you don't have one, install MinGW-w64 and make sure it's added to your system's PATH.
2.  **Open a command prompt:** Navigate to the root directory of the project.
3.  **Compile the code:** Run the following command:

    ```bash
    g++ -std=c++17 -o key.exe main.cpp crypto/sha256.cpp
    ```

4.  **Run the executable:** Once compiled, you can run `key.exe` from the command prompt.

---
*Note: The original `key.bat` script is still included for historical reference but is not recommended for use due to its lack of real security.*
