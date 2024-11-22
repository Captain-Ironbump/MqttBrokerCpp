#include <string>  // Include the string library
#include <iostream>  // Include the iostream library
#include <unistd.h>
#include "../include/Client.hpp"
#include "../include/Broker.hpp"  // Include the Broker header file

#include "../include/socketUtil.h"
#include "../include/logger.hpp"

int main() {
    Logger logger("");

    Client* client1 = new Client("client1");  // Create a new client object
    Client* client2 = new Client("client2");  // Create a new client object
    
    Broker* broker = new Broker(logger, "", 2000);  // Create a new broker object
    
    broker->addClient("client1", client1);
    broker->addClient("client2", client2);

    broker->printClients(logger);



    /*close(clientSocketFD);*/
    /*shutdown(serverSocketFD, SHUT_RDWR);*/

    return 0;
}

