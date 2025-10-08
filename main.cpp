#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <set>
#include <map>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

// Windows related libraries
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <iphlpapi.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "iphlpapi.lib")

// Linux related libraries
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <ifaddrs.h>
    #include <fstream>
    #include <csignal>
    #include <filesystem>
    #include <regex>
    namespace fs = std::filesystem;
    #include <fcntl.h>
#endif

// Helper: Split string by delimiter
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) elems.push_back(item);
    return elems;
}

// 1. Remote IP Reachability
bool is_ip_reachable(const std::string& ip, int port = 80) {
#ifdef _WIN32
    std::string cmd = "ping -n 1 -w 1000 " + ip + " > nul 2>&1";
    return system(cmd.c_str()) == 0;
#else
    std::string cmd = "ping -c 1 -W 1 " + ip + " > /dev/null 2>&1";
    if (system(cmd.c_str()) == 0) return true;
    return false;
#endif
}

// 2. Get local IPv4 addresses
std::vector<std::string> get_local_ips() {
    std::vector<std::string> ips;
#ifdef _WIN32
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_INET; // IPv4 only
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES adapters = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);


    if (GetAdaptersAddresses(family, flags, NULL, adapters, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(adapters);
        adapters = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
    }

    // GetAdaptersAddresses is a Windows API that returns detailed info about all network interfaces.
    // It returns the list of adapter and it stored in the adapters, adapters will be the head of the linkedlist
    if (GetAdaptersAddresses(family, flags, NULL, adapters, &outBufLen) == NO_ERROR) {

        // We are travercing to each adapters and find the ipv4 addresses.
        PIP_ADAPTER_ADDRESSES adapter = adapters;
        while (adapter) {
            PIP_ADAPTER_UNICAST_ADDRESS ua = adapter->FirstUnicastAddress;
            while (ua) {
                //Convert binary IP to readable string
                char buf[INET_ADDRSTRLEN];
                sockaddr_in* sa_in = (sockaddr_in*)ua->Address.lpSockaddr;
                inet_ntop(AF_INET, &sa_in->sin_addr, buf, sizeof(buf));
                ips.push_back(buf);
                ua = ua->Next;
            }
            adapter = adapter->Next;
        }
    }
    free(adapters);
    
#else
    struct ifaddrs *ifaddr, *ifa;
    // Populates ifaddr with a linked list of network interfaces.
    if (getifaddrs(&ifaddr) == -1) return ips;
    ifa = ifaddr;
    // Travercing the linked list the get Ipv4 addresses
    while (ifa) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            ////Convert binary IP to readable string
            char buf[INET_ADDRSTRLEN];
            struct sockaddr_in* sa_in = (struct sockaddr_in*)ifa->ifa_addr;
            inet_ntop(AF_INET, &sa_in->sin_addr, buf, sizeof(buf));
            ips.push_back(buf);
        }
        ifa = ifa->ifa_next;
    }
    freeifaddrs(ifaddr);
#endif
    return ips;
}

// 3. Check if two IPs are in the same /24 subnet
bool same_subnet(const std::string& ip1, const std::string& ip2) {
    auto a = split(ip1, '.');
    auto b = split(ip2, '.');
    return a.size() == 4 && b.size() == 4 && a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}


void check_ports(const std::vector<int>& ports, bool do_kill = false) {
#ifdef _WIN32
    // Iterating through all ports and find the process that are using this port
    for (int port : ports) {
        std::string cmd = "netstat -ano | findstr :" + std::to_string(port) + " > tmp.txt";
        system(cmd.c_str());

        std::ifstream file("tmp.txt");
        std::string line;
        bool found = false;
        while (getline(file, line)) {
            found = true;

            // prints protocol, local addr, foreign addr, state, PID
            std::cout << line << "\n"; 

            if (do_kill) {
                std::istringstream iss(line);
                std::string proto, local, remote, state, pid_str;
                iss >> proto >> local >> remote >> state >> pid_str;

                try {
                    int pid = std::stoi(pid_str);
                    std::string kill_cmd = "taskkill /PID " + std::to_string(pid) + " /F > nul 2>&1";
                    if (system(kill_cmd.c_str()) == 0)
                        std::cout << "  -> Process killed successfully.\n";
                    else
                        std::cout << "  -> Failed to kill process.\n";
                } catch (...) {
                    std::cout << "  -> Could not parse PID.\n";
                }
            }
        }
        if (!found) std::cout << "Port " << port << " is IDLE\n";
        file.close();
        remove("tmp.txt");
    }

#else
    for (int port : ports) {
        std::cout << "\nChecking port " << port << "...\n";

        std::string cmd = "sudo lsof -iTCP:" + std::to_string(port) + " -sTCP:LISTEN > tmp.txt";
        system(cmd.c_str());

        std::ifstream file("tmp.txt");
        std::string line;
        bool found = false;

        getline(file, line);
        while (getline(file, line)) {
            found = true;
            std::cout << line << "\n";

            if (do_kill) {
                std::istringstream iss(line);
                std::string pname;
                int pid;
                // extract command name and PID
                iss >> pname >> pid; 

                if (kill(pid, SIGKILL) == 0)
                    std::cout << "  -> Process killed successfully.\n";
                else
                    std::cout << "  -> Failed to kill process (maybe need sudo).\n";
            }
        }

        if (!found) std::cout << "Port " << port << " is IDLE\n";

        file.close();
        remove("tmp.txt");
    }

#endif
}

int main(int argc, char* argv[]) {

    std::string remote_ip;
    std::cout << "Enter remote IP: ";
    std::cin >> remote_ip;

    // 1. Reachability
    std::cout << "Checking reachability...\n";
    if (is_ip_reachable(remote_ip))
        std::cout << "Remote IP is REACHABLE.\n";
    else
        std::cout << "Remote IP is UNREACHABLE.\n";

    // 2. Local IP and subnet check
    auto local_ips = get_local_ips();
    bool same = false;
    std::cout << "Local IPs: ";
    for (auto& ip : local_ips) std::cout << ip << " ";
    std::cout << "\n";
    for (auto& ip : local_ips) {
        if (same_subnet(ip, remote_ip)) same = true;
    }
    std::cout << (same ? "Same network\n" : "Different network\n");

    // 3. Port status
    std::cout << "Enter ports to check (comma separated, e.g. 22,80,443): ";
    std::string portline;
    std::cin >> portline;
    auto portstrs = split(portline, ',');
    std::vector<int> ports;
    for (auto& s : portstrs) ports.push_back(std::stoi(s));
    bool do_kill = false;
    if (argc > 1 && std::string(argv[1]) == "--kill") do_kill = true;
    check_ports(ports, do_kill);

    return 0;
}