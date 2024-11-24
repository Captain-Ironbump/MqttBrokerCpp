#include "../include/Broker.hpp" // Include the Broker header file
#include "../include/Client.hpp" // Include the Client header file
#include "../include/socketUtil.h" // Include the Logger header file

#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>

using namespace std; // Use the standard namespace

// Constructor
Broker::Broker(Logger& logger, const string& serverIP, const int& serverPort) 
  : logger(logger) , running(false) , serverIP(serverIP) 
  , serverPort(serverPort) 
{
  this->clients = unordered_map<string, Client*>();
  this->serverSocketFD = 0;
}

// Destructor
Broker::~Broker() 
{
  this->stop();
  for (auto& pair : this->clients) {
    delete pair.second;
  }
  this->clients.clear();
}

// Connection connectionHandler
void Broker::connectionHandler()
{
  logger.log(LogLevel::INFO, "Connection handler started");
  fd_set current_sockets, ready_sockets;
  
  struct timeval tv;

  FD_ZERO(&current_sockets);
  FD_SET(this->serverSocketFD, &current_sockets);

  tv.tv_sec = 5;
  tv.tv_usec = 0;


  while (this->running)
  {  
    logger.log(LogLevel::INFO, "Waiting for client connection...");

    ready_sockets = current_sockets;

    int activity = select(this->serverSocketFD + 1, &ready_sockets, NULL, NULL, &tv);
    if (activity == 0)
    {
      logger.log(LogLevel::ERROR, "Timeout while waiting for client connection");
      break;
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

    logger.log(LogLevel::INFO, "Accepted connection from client");
        // handle client CONNECT packet with client ID and insert into clients map with clients file descriptor
        // TODO: Implement this 
  }  
  logger.log(LogLevel::INFO, "Connection handler stopped");
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
      logger.log(LogLevel::ERROR, "Failed to bind server socket");
      return;
    }
    logger.log(LogLevel::INFO, "Server socket bound successfully");

    int res_listen = listen(serverSocketFD, 5);
    if (res_listen < 0) 
    {
      logger.log(LogLevel::ERROR, "Failed to listen on server socket");
      return;
    }
    this->serverSocketFD = serverSocketFD;

    this->running = true;
    logger.log(LogLevel::INFO, "Starting broker thread");
    this->connectionThread = thread(&Broker::connectionHandler, this);

    return;
  }
  logger.log(LogLevel::ERROR, "Broker already running");
}

// Stop the Broker
void Broker::stop() 
{
  if (this->running) 
  {
    this->running = false;
    logger.log(LogLevel::INFO, "Trying to stop broker thread");
    if (connectionThread.joinable()) 
    {
      logger.log(LogLevel::INFO, "Joining broker thread");
      connectionThread.join();
      for (auto& pair : this->clients) 
      {
        pair.second->closeFileDescriptor();
      }
      shutdown(this->serverSocketFD, SHUT_RDWR);
      return;
    }
    logger.log(LogLevel::ERROR, "Broker thread not joinable");
    return;
  }
  logger.log(LogLevel::ERROR, "Broker not running");
}

// Add a client to the broker
void Broker::addClient(string client_id, Client* client) 
{
  logger.log(LogLevel::INFO, "Adding client: " + client_id);
  this->clients[client_id] = client;
}

// Remove a client from the broker
void Broker::removeClient(string client_id) 
{
  logger.log(LogLevel::INFO, "Removing client: " + client_id);
  this->clients.erase(client_id);
}

// Print all clients
void Broker::printClients(Logger& logger) 
{
  for (auto const& x : this->clients) 
  {
    logger.log(LogLevel::INFO, "Client ID: " + x.first + " Object[" + x.second->to_string() + "]");
  }
}
