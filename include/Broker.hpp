#ifndef BROKER_H
#define BROKER_H

#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include "Client.hpp"
#include "UUID.hpp"
#include "logger.hpp"

enum ClientsReconnectionStatusCodes {
  RECONNECTED_CLIENT = 1,
  NEW_CLIENT = 0,
  FAILED_SCOKET_READ = -1,
  FAILED_GENERAL = -2
};

class Broker
{
private:
  Logger& logger;
  int serverSocketFD;
  const std::string& serverIP;
  const int& serverPort;
  std::unordered_map<UUID, std::shared_ptr<Client>> clients;
  std::mutex clientsMutex;
  std::mutex runningMutex;
  std::condition_variable runningCV;
  std::atomic<bool> running;
  std::thread connectionThread;
  std::thread cleanUpThread;
  std::vector<std::unique_ptr<std::thread>> clientThreads; 
  bool stopRequested = false;

  void clientReconnectionHandler(int clientSocketFD, std::shared_ptr<Client>& client, int* r_statusCode);
  void connectionHandler();
  void clientConnectionHandler(std::shared_ptr<Client> client);
  void cleanUpDisconnectedClientsThread();
public:
  Broker(Logger& logger, const std::string& serverIP, const int& serverPort);
  ~Broker();
  void start();
  void stop();
  void addClient(UUID client_id, std::shared_ptr<Client>);
  void removeClient(UUID client_id);
  void printClients(Logger& logger);
  void joinAllThreads();
};

#endif // ndef BROKER_H
