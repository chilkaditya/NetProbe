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
