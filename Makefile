
# Windows Volume Limiter Makefile for MinGW-w64 g++
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -DUNICODE -D_UNICODE -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS = -mwindows -static-libgcc -static-libstdc++ -lole32 -loleaut32 -luuid -lcomctl32

TARGET = VolumeLimiter.exe
SOURCES = main.cpp MainWindow.cpp AudioController.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OBJECTS) $(TARGET) 2>nul 2>&1 || true
