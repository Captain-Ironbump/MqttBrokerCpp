CXX = g++
CXXFLAGS = -Wall -g -Iinclude -std=c++17
LDFLAGS = -Llib -lsocketutil -lloggercpp

SOURCES = src/main.cpp src/Client.cpp src/Broker.cpp src/UUID.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = my_program

LIB_DIR = lib
LIB_NAMES = libsocketutil libloggercpp

ifeq ($(OS),Windows_NT)
TARGET = my_program.exe
CLEAN_CMD = del /f
CLEAN_FOLDER_FORCED = rmdir /s /q
MOVE_CMD = move
else
CLEAN_CMD = rm -f
CLEAN_FOLDER_FORCED = rm -rf
MOVE_CMD = mv
endif


# Ensure the 'lib' directory exists
$(shell if not exist lib mkdir lib)

LIB_SOCKETUTIL_EXISTS = $(wildcard $(LIB_DIR)/libsocketutil.a)
LIB_LOGGERCPP_EXISTS = $(wildcard $(LIB_DIR)/libloggercpp.a)

ifeq ($(LIB_SOCKETUTIL_EXISTS),)
$(info static libsocketutil.a does exist)
	git clone https://github.com/Captain-Ironbump/SocketUtilcpp.git 
	cd SocketUtilcpp && make && $(MOVE_CMD) libsocketutil.a ../lib/ && cd ..
	$(CLEAN_FOLDER_FORCED) SocketUtilcpp
endif
ifeq ($(LIB_LOGGERCPP_EXISTS),)
$(info static libloggercpp.a does exist)
	git clone https://github.com/Captain-Ironbump/LoggerUtilcpp.git 
	cd LoggerUtilcpp && make && $(MOVE_CMD) libloggercpp.a ../lib/ && cd ..
	$(CLEAN_FOLDER_FORCED) LoggerUtilcpp
endif


$(TARGET): $(OBJECTS) 
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)
	$(CLEAN_CMD) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(CLEAN_CMD) $(TARGET)

remove-lib:
	$(CLEAN_CMD) lib/**

remove-github-lib:
	$(CLEAN_FOLDER_FORCED) SocketUtilcpp/