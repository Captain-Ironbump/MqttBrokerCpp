#ifndef BROKER_H
#define BROKER_H

#include <unordered_map>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include "Client.hpp"
#include "logger.hpp"

class Broker
{
private:
  Logger& logger;
  int serverSocketFD;
  const std::string& serverIP;
  const int& serverPort;
  std::unordered_map<std::string, Client*> clients;
  std::mutex clientsMutex;
  std::atomic<bool> running;
  std::thread connectionThread;

  void connectionHandler();
public:
  Broker(Logger& logger, const std::string& serverIP, const int& serverPort);
  ~Broker();
  void start();
  void stop();
  void addClient(std::string client_id, Client* client);
  void removeClient(std::string client_id);
  void printClients(Logger& logger);
  void joinAllThreads();
};

#endif // ndef BROKER_H
