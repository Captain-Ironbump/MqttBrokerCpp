#include "../include/Client.hpp"

using namespace std;

// Constructor
Client::Client(string client_id) {
  this->client_id = client_id;
}

// convert the client to a string
string Client::to_string() {
  return this->client_id;
}
