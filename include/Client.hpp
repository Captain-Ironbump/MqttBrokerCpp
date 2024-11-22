#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <chrono>

using namespace std;

enum ClientStatus {
  CONNECTED,
  DISCONNECTED
};

class Client {
private:
  string client_id;
  chrono::duration<double> keep_alive;
  chrono::time_point<chrono::system_clock> last_seen;
  ClientStatus status;

public:
  Client(string client_id);
  string to_string();
};

#endif // !CLIENT_H

