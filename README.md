# Secret-Folder
It creates a hidden, secret folder via the batch file.

I mean... It's pretty simple. You can figure it out by yourself I guess.

**BUT**, in case if you still need to read some instructions to understand what this file does, here it is;

1. Open it up
2. It will create a folder called "Private"
3. Open the batch file again. By pressing "Y" and pressing the `Enter` button, you can hide the folder.
4. To unlock your hidden folder, you **have to** use the batch file and the set password in order to access your hidden folder once again.

---

> "What if I lose my batch file?"
- No worries! You can download it from here back again in order to access your folder.

> "How can I change the password?"
- Find the exact piece of line in the batch file (the example have provided down below)
```batch
REM You can set your password from below this comment
if NOT %pass%== ballsofsigma123 goto FAIL
REM You can set your password from above this comment
```
Just change "ballsofsigma123" to any password you want.
