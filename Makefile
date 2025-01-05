CXX = g++
CXXFLAGS = -Wall -g -Iinclude -std=c++17
LDFLAGS = -Llib -lsocketutil -lloggercpp

SOURCES = src/main.cpp src/Client.cpp src/Broker.cpp src/UUID.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = my_program

LIB_DIR = lib
LIB_REPO_URLS = https://github.com/Captain-Ironbump/SocketUtilcpp.git https://github.com/Captain-Ironbump/LoggerUtilcpp.git
LIB_NAMES = libsocketutil libloggercpp

$(LIB_DIR)/$(LIB_NAMES): 
	$(foreach url, $(LIB_REPO_URLS), $(shell git clone $(url)))


$(TARGET): $(OBJECTS) 
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)
	rm -f $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)

remove-lib:
	rm -f lib/**