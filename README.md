# NetProbe

### Overview
##### NetProbe is a lightweight C++ command-line tool that helps you:

- Check if a remote IP is reachable.
- Display all local IPv4 addresses.
- Determine whether a remote IP is on the same subnet.
- Inspect TCP ports (to see which process is listening).
- Works on Windows, Linux. 

### Requirements
##### Windows
- MSVC or MinGW compiler
- Administrator privileges not required

#### Linux
- g++ compiler
- May require sudo to list all processes (for lsof and netstat)

### Build & Run
```shell
git clone <repo-url>
cd <folder-name>
make  #build
./netprobe # for Linux
./netprobe.exe # for Windows
```
### Sample Output
Sample working video & Some images of output - https://drive.google.com/file/d/1lmZEmEoL8_7FjMKzlHCtW6OrpQMtdWFt/view?usp=sharing

<img width="617" height="242" alt="image" src="https://github.com/user-attachments/assets/a6e76642-9cc3-4352-9868-bbeb68ae5e94" />
<img width="625" height="270" alt="image" src="https://github.com/user-attachments/assets/d18ec6f1-2d05-41f1-8ed5-a7a6bb1c73b9" />



