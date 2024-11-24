#include <string>  // Include the string library
#include <unistd.h>
#include "../include/Client.hpp"
#include "../include/Broker.hpp"  // Include the Broker header file

#include "../include/logger.hpp"

int main() {
    Logger logger("");

    Client* client1 = new Client("client1");  // Create a new client object
    Client* client2 = new Client("client2");  // Create a new client object
    
    Broker* broker = new Broker(logger, "127.0.0.1", 1883);  // Create a new broker object

    broker->start(); 

    std::this_thread::sleep_for(std::chrono::seconds(10));

    delete broker;
    
    return 0;
}

