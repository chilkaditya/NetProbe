CXX = g++
CXXFLAGS = -std=c++17

SRC = main.cpp

ifeq ($(OS),Windows_NT)
	TARGET = netprobe.exe
	LDFLAGS = -lws2_32 -liphlpapi
else
	TARGET = netprobe
	UNAME_S := $(shell uname -s)
    # Uncomment the next line if you use GCC 8 or earlier
    # LDFLAGS = -lstdc++fs
endif

all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f netprobe netprobe.exe