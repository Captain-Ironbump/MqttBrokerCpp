CXX = g++
CXXFLAGS = -Wall -g -Iinclude -std=c++17
LDFLAGS = -Llib -lsocketutil -lloggercpp

SOURCES = src/main.cpp src/Client.cpp src/Broker.cpp src/UUID.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = my_program

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)
	rm -f $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)
