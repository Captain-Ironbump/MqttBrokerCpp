#include "../include/Broker.hpp" // Include the Broker header file
#include "../include/Client.hpp" // Include the Client header file
#include "../include/socketUtil.h" // Include the Logger header file

#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <iostream> // Include the iostream library
#include <unistd.h> // For close()

using namespace std; // Use the standard namespace

// Constructor TODO: maybe add a timeout for the connection handler
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
        break;
      }
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

    logger.log(LogLevel::INFO, "Accepted connection from client");
    thread clientThread = thread(&Broker::clientConnectionHandler, this, ref(clientSocketFD));
    this->clientThreads.push_back(make_unique<thread>(std::move(clientThread)));
  }  
  logger.log(LogLevel::INFO, "Connection handler stopped");
}

// Client connection handler
void Broker::clientConnectionHandler(const int& clientSocketFD) 
{
  logger.log(LogLevel::INFO, "Client connection handler started");
  // TODO: replace current with reading in MQTT packet
  char buffer[1024] = {0};
  int valread = read(clientSocketFD, buffer, 1024);
  if (valread < 0) 
  {
    logger.log(LogLevel::ERROR, "Failed to read from client socket");
    return;
  }
  logger.log(LogLevel::INFO, "Received message from client: " + string(buffer));
  string response = "Hello from broker";
  send(clientSocketFD, response.c_str(), response.size(), 0);
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

    this->running = true;
    this->stopRequested = false;

    this->serverSocketFD = serverSocketFD;

    logger.log(LogLevel::INFO, "Starting broker thread");
    this->connectionThread = thread(&Broker::connectionHandler, this);

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

void Broker::joinAllThreads()
{
  this->logger.log(LogLevel::INFO, "Joining all threads");
  if (this->connectionThread.joinable())
  {
    this->connectionThread.join();
    this->logger.log(LogLevel::INFO, "Connection thread joined");
  }
}
