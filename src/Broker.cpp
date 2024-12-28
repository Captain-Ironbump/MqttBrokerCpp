#include "../include/Broker.hpp" // Include the Broker header file
#include "../include/Client.hpp" // Include the Client header file
#include "../include/socketUtil.h" // Include the Socket Helper header file
#include "../include/UUID.hpp"

#include <cstring>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <iostream> // Include the iostream library
#include <unistd.h> // For close()
#include <cerrno> // For errno
// include vector
#include <vector>

using namespace std; // Use the standard namespace

// Constructor TODO: maybe add a timeout for the connection handler
Broker::Broker(Logger& logger, const string& serverIP, const int& serverPort) 
  : logger(logger) , running(false) , serverIP(serverIP) 
  , serverPort(serverPort), serverSocketFD(0), clients() 
{
  
}

// Destructor
Broker::~Broker() 
{
  this->stop();
  this->clients.clear();
}

// Connection connectionHandler
void Broker::connectionHandler()
{
  logger.log(LogLevel::INFO, "Connection handler started");
  fd_set current_sockets, ready_sockets;
  
  struct timeval current_time, ready_time;

  FD_ZERO(&current_sockets);
  FD_SET(this->serverSocketFD, &current_sockets);

  current_time.tv_sec = 5;
  current_time.tv_usec = 0;


  while (true)
  {
    {
      std::unique_lock<std::mutex> lock(this->runningMutex);
      if (this->stopRequested)
      {
        logger.log(LogLevel::INFO, "Stop requested");
        lock.unlock();
        break;
      }
    }

    // This can be done periodically, instead of joining inside the loop
    // Only join threads that are joinable (this avoids blocking the loop)
    for (auto it = this->clientThreads.begin(); it != this->clientThreads.end();)
    {
      if (it->get()->joinable())
      {
        it->get()->join();  // Join the thread if it is joinable
      }
      it = this->clientThreads.erase(it); // Remove the thread from the list
    }
    logger.log(LogLevel::INFO, "Waiting for client connection...");

    ready_time = current_time;
    ready_sockets = current_sockets;

    int activity = select(this->serverSocketFD + 1, &ready_sockets, NULL, NULL, &ready_time);
    if (activity == 0)
    {
      logger.log(LogLevel::ERROR, "Timeout while waiting for client connection");
      continue;
    }
    if (activity < 0)
    {
      logger.log(LogLevel::ERROR, "Failed to select");
      continue;
    }

    if (FD_ISSET(this->serverSocketFD, &ready_sockets) == 0)
    {
      logger.log(LogLevel::ERROR, "Server socket not set");
      continue;
    }

    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(this->serverSocketFD, (struct sockaddr *)&clientAddress, &clientLen);
    if (clientSocketFD < 0) 
    {
      logger.log(LogLevel::ERROR, "Failed to accept connection");    
      continue;
    }

    const char *ip = inet_ntoa(clientAddress.sin_addr);
    unsigned short port = ntohs(clientAddress.sin_port);
    char ipBuffer[22];
    snprintf(ipBuffer, 22, "%s:%d", ip, port);

    // check if the client has already connected before
    // first sended message should be the client UUID or NULL if new client
    int* statusCode = new int(0);
    std::shared_ptr<Client> client = nullptr;
    
    clientReconnectionHandler(clientSocketFD, client, statusCode);
    if (*statusCode < 0)
    {
      logger.log(LogLevel::ERROR, "Failed to reconnect client");
      continue;
    }
    if (*statusCode == ClientsReconnectionStatusCodes::NEW_CLIENT)
    {
      // Create the clients UUID 
      UUID uuid = UUID();

      // Create the client Object
      client = std::make_shared<Client>(uuid, clientSocketFD);
      this->addClient(uuid, client);
    }

    /*logger.log(LogLevel::INFO, "Client created with ID: %d", client.getClientID().getUUID());*/

    logger.log(LogLevel::INFO, "Accepted connection from client [%s] with FD: %d", ipBuffer, clientSocketFD);
    if (client == nullptr)
    {
      logger.log(LogLevel::CRITICAL, "Client is null");
      continue;
    }
    // Create a new thread to handle the client connection
    std::unique_ptr<std::thread> clientThread = std::make_unique<std::thread>(&Broker::clientConnectionHandler, this, client);
    this->clientThreads.push_back(std::move(clientThread));
    this->clientThreads.back()->detach(); // Detach the thread to run independently
  }   
  logger.log(LogLevel::INFO, "Connection handler stopped");
}

// Client reconnection handler
void Broker::clientReconnectionHandler(int clientSocketFD, std::shared_ptr<Client>& r_client, int* r_statusCode)
{
  char buffer[21] = {0};
  int valread = read(clientSocketFD, buffer, 21);
  if (valread < 0) 
  {
    logger.log(LogLevel::ERROR, "Failed to read id from client socket");
    *r_statusCode = valread;
    return;
  }
  std::string stringBuffer = std::string(buffer);
  if (stringBuffer.compare("NULL") == 0)
  {
    logger.log(LogLevel::INFO, "Client is new");
    *r_statusCode = ClientsReconnectionStatusCodes::NEW_CLIENT;
    return;
  }
  for (auto& pair : this->clients) 
  {
    std::unique_lock<std::mutex> lock(this->clientsMutex);
    if (pair.second->getClientID().getUUID() == std::stoull(stringBuffer))
    {
      logger.log(LogLevel::INFO, "Client is not new with id: %s", stringBuffer.c_str());
      r_client = pair.second;
      r_client->setStatus(ClientStatus::CONNECTED);
      r_client->setClientSocketFD(clientSocketFD);
      *r_statusCode = ClientsReconnectionStatusCodes::RECONNECTED_CLIENT;
      return;
    }
    lock.unlock();
  } 
}

// Client connection handler
void Broker::clientConnectionHandler(std::shared_ptr<Client> client)
{
  logger.log(LogLevel::INFO, "Client connection handler started");
  // TODO: replace current with reading in MQTT packet

  // almost no need to check if client is connected  
  if (client->getStatus() == ClientStatus::DISCONNECTED)
  {
    logger.log(LogLevel::INFO, "Client %llu is already disconnected", client->getClientID().getUUID());
    return;
  }

  // Client Object
  /*std::unique_ptr<Client> client = new Client();*/
  char buffer[1024] = {0};
  while (true) 
  {
    {
      std::unique_lock<std::mutex> lock(this->runningMutex);
      if (this->stopRequested)
      {
        logger.log(LogLevel::INFO, "Stop requested");
        lock.unlock();
        break;
      }
    }

    int valread = read(client->getClientSocketFD(), buffer, 1024);
    
    if (valread == 0) 
    {
      std::unique_lock<std::mutex> lock(this->clientsMutex);
      client->setStatus(ClientStatus::DISCONNECTED);
      client->updateLastSeen();
      logger.log(LogLevel::INFO, "Client disconnected with id: %llu", client->getClientID().getUUID());
      lock.unlock();
      break;
    }

    if (valread < 0) 
    {
      logger.log(LogLevel::ERROR, "Failed to read from client socket");
      break;
    }
    logger.log(LogLevel::INFO, "Received message from client: %s", buffer);
    string response = "Hello from broker with id: " + to_string(client->getClientID().getUUID());
    send(client->getClientSocketFD(), response.c_str(), response.size(), 0);
  }
  /*logger.log(LogLevel::INFO, "Cleaning up Client with id: %llu", client->getClientID().getUUID());*/
  /*this->removeClient(client->getClientID());*/
  logger.log(LogLevel::INFO, "Client connection handler stopped");
}

// Start the broker
void Broker::start() 
{
  if (!this->running) 
  {
    // create a server Socket
    int serverSocketFD = createTCPIPv4Socket();
    if (serverSocketFD < 0) 
    {
      logger.log(LogLevel::ERROR, "Failed to create server socket");
      return;
    }

    int opt = 1;
    if (setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      perror("setsockopt failed");
      return;
    }

    logger.log(LogLevel::INFO, "Server socket created successfully");
    sockaddr_in* serverAddress = createIPv4SockAddr(this->serverIP.c_str(), this->serverPort);
    if (serverAddress == nullptr) 
    {
      logger.log(LogLevel::ERROR, "Failed to create server address");
      return;    
    }
    logger.log(LogLevel::INFO, "Server address created successfully");
    int res = bind(serverSocketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));
    if (res < 0) 
    {
      string error = "Failed to bind server socket with: " + string(strerror(errno));
      logger.log(LogLevel::ERROR, error.c_str());
      return;
    }
    logger.log(LogLevel::INFO, "Server socket bound successfully");

    int res_listen = listen(serverSocketFD, 5);
    if (res_listen < 0) 
    {
      logger.log(LogLevel::ERROR, "Failed to listen on server socket");
      return;
    }

    this->running = true;
    this->stopRequested = false;

    this->serverSocketFD = serverSocketFD;

    logger.log(LogLevel::INFO, "Starting broker thread");
    this->connectionThread = thread(&Broker::connectionHandler, this);
    this->cleanUpThread = thread(&Broker::cleanUpDisconnectedClientsThread, this);
    return;
  }
  logger.log(LogLevel::WARNING, "Broker already running");
}

// Stop the Broker
void Broker::stop() 
{
  if (this->running) 
  {
    {
      std::unique_lock<std::mutex> lock(this->runningMutex);
      this->stopRequested = true;
      this->runningCV.notify_all();
      lock.unlock();
    }

    this->joinAllThreads();
    
    for (auto& pair : this->clients) 
    {
      pair.second->closeFileDescriptor();
    }
    this->logger.log(LogLevel::INFO, "Closing server socket");
    shutdown(this->serverSocketFD, SHUT_RDWR);
    close(this->serverSocketFD);
    this->logger.log(LogLevel::INFO, "Server socket closed");
    this->running = false;
    return;
  }
  logger.log(LogLevel::WARNING, "Broker not running");
}

// Add a client to the broker
void Broker::addClient(UUID client_id, std::shared_ptr<Client> client)
{
  logger.log(LogLevel::INFO, "Adding client: %llu", client_id.getUUID());
  std::unique_lock<std::mutex> lock(this->clientsMutex);
  this->clients[client_id] = client; 
  lock.unlock();
}

// Remove a client from the broker
void Broker::removeClient(UUID client_id) 
{
  logger.log(LogLevel::INFO, "Removing client: %llu", client_id.getUUID());
  std::unique_lock<std::mutex> lock(this->clientsMutex);
  auto id = this->clients.find(client_id); 
  if (id != this->clients.end()) 
  {
    this->clients.erase(id);
  }
  lock.unlock();
}

// Print all clients
void Broker::printClients(Logger& logger) 
{
  for (auto const& x : this->clients) 
  {
    logger.log(LogLevel::INFO, "Client ID: %llu Object[%s]", x.first.getUUID(), x.second->to_string().c_str());
  }
}

void Broker::joinAllThreads()
{
  this->logger.log(LogLevel::INFO, "Joining all threads");
  if (this->connectionThread.joinable())
  {
    this->connectionThread.join();
    this->logger.log(LogLevel::INFO, "Connection thread joined");
  }
  if (this->cleanUpThread.joinable())
  {
    logger.log(LogLevel::INFO, "Joining clean up thread");
    this->cleanUpThread.join();
    this->logger.log(LogLevel::INFO, "Clean up thread joined");
  }
}

// clean up disconnected clients thread
// make this thread onyl run every second
void Broker::cleanUpDisconnectedClientsThread()
{
  logger.log(LogLevel::INFO, "Clean up disconnected clients thread started");
  while (true)
  {
    {
      std::unique_lock<std::mutex> lock(this->runningMutex);
      if (this->stopRequested)
      {
        logger.log(LogLevel::INFO, "Stop requested");
        lock.unlock();
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Get the current time 
    auto currentTime = std::chrono::system_clock::now();

    // Temporary vector to hold the IDs of clients to be removed
    std::vector<UUID> clientsToRemove;

    // Iterate through the clients and check for cleanup conditions
    {
      for (auto& pair : this->clients)
      {
        // If client is disconnected and keep-alive time has expired
        if (pair.second->getStatus() == ClientStatus::DISCONNECTED && pair.second->getKeepAlive() + pair.second->getLastSeen() < currentTime)
        {
          logger.log(LogLevel::INFO, "Marking client for cleanup with ID: %llu", pair.first.getUUID());
          clientsToRemove.push_back(pair.first);  // Mark client for removal
        }
      }
    }

    // Now that iteration is complete, safely remove clients
    {
      for (const auto& clientId : clientsToRemove)
      {
        logger.log(LogLevel::INFO, "Cleaning up Client with id: %llu", clientId.getUUID());
        this->removeClient(clientId);
      }
    }
  }

  logger.log(LogLevel::INFO, "Clean up disconnected clients thread stopped");
}

