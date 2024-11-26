#include <string>  // Include the string library
#include <unistd.h>
#include <iostream>  // Include the iostream library
#include "../include/Broker.hpp"  // Include the Broker header file

#include "../include/logger.hpp"

int main() {
    Logger logger("");
    std::string command;

    Broker* broker = new Broker(logger, "127.0.0.1", 1883);  // Create a new broker object
    
    std::cout << "Welcome to the Broker CLI. Type 'start' to start the broker, 'stop' to stop it and 'exit' to exit the program" << std::endl;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        if (command == "exit") {
            logger.log(LogLevel::INFO, "Exiting program");
            break;
        }
        else if (command == "start") {
            broker->start();
        }
        else if (command == "stop") {
            broker->stop();
        }
    }

    delete broker;
    return 0;
}

