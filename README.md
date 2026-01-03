# Secret-Folder
It creates a hidden, secret folder via a batch file.

I mean... It's pretty simple thing. You can figure some things out by yourself I guess.

**BUT** I'm still gonna tell you basically what you are gonna do with it

1. Open it up
2. It will create a folder called "Private"
3. Open the batch file again. By pressing "Y" and hitting enter button you can hide the folder
4. To unlock your hidden folder back again you can use the batch file and use your password in order to access your hidden folder again.

---

> "What if I lose my batch file?"
- No worries! You can download it from here again so you can access your hidden folder.

> "How can I change the password?"
- Find the line from down below in the batch file
```batch
REM You can set your password from below this comment
if NOT %pass%== ballsofsigma123 goto FAIL
REM You can set your password from above this comment
```
Just change "ballsofsigma123" to anything you want.

---

## Building the Executable (C++)

This repository now includes a C++ version of the original batch script. To compile it into an executable (`.exe`) on a Windows machine with a C++ compiler (like MinGW-w64), follow these steps:

1.  **Install a C++ compiler:** If you don't have one, you can install MinGW-w64.
2.  **Open a command prompt:** Navigate to the directory containing `main.cpp`.
3.  **Compile the code:** Run the following command:

    ```bash
    g++ -o key.exe main.cpp
    ```

4.  **Run the executable:** Once compiled, you can run `key.exe` from the command prompt.
