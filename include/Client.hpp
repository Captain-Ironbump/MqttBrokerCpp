#ifndef CLIENT_H
#define CLIENT_H

#include "UUID.hpp"

#include <mutex>
#include <string>
#include <chrono>
#include <set>

using namespace std;

enum ClientStatus {
  CONNECTED,
  DISCONNECTED
};

class Client {
private:
  UUID client_id;
  chrono::duration<double> keep_alive;
  chrono::time_point<chrono::system_clock> last_seen;
  ClientStatus status;
  int clientSocketFD;
  set<string> subscriptions;

  mutable mutex subscriptionMutex;
public:
  Client(UUID client_id, int clientSocketFD = -1, chrono::duration<double> keep_alive = chrono::seconds(10));
  ~Client();
  string to_string();
  void setClientSocketFD(int clientSocketFD);
  int getClientSocketFD();
  void setKeepAlive(chrono::duration<double> keep_alive);
  chrono::duration<double> getKeepAlive();
  void updateLastSeen();
  chrono::time_point<chrono::system_clock> getLastSeen();
  void setStatus(ClientStatus status);
  ClientStatus getStatus();

  bool isAlive();

  void addSubscription(const string& topic);
  void removeSubscription(const string& topic);
  bool hasSubscription(const string& topic) const;
  set<string> getSubscriptions() const;

  UUID getClientID() const;

  void closeFileDescriptor();
};

#endif // !CLIENT_H

