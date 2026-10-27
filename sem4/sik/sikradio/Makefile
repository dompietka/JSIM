CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS = -lssl -lcrypto

TARGET = sikradio
SOURCES = sikradio.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)