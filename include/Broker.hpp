#ifndef BROKER_H
#define BROKER_H

#include <condition_variable>
#include <unordered_map>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include "Client.hpp"
#include "UUID.hpp"
#include "logger.hpp"

class Broker
{
private:
  Logger& logger;
  int serverSocketFD;
  const std::string& serverIP;
  const int& serverPort;
  std::unordered_map<UUID, Client*> clients;
  std::mutex clientsMutex;
  std::mutex runningMutex;
  std::condition_variable runningCV;
  std::atomic<bool> running;
  std::thread connectionThread;
  std::vector<std::unique_ptr<std::thread>> clientThreads; 
  bool stopRequested = false;

  void clientReconnectionHandler(int clientSocketFD, Client*& r_client, int* r_statusCode);
  void connectionHandler();
  void clientConnectionHandler(Client* client);
public:
  Broker(Logger& logger, const std::string& serverIP, const int& serverPort);
  ~Broker();
  void start();
  void stop();
  void addClient(UUID client_id, Client* client);
  void removeClient(UUID client_id);
  void printClients(Logger& logger);
  void joinAllThreads();
};

#endif // ndef BROKER_H
