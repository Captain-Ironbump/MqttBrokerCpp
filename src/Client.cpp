#include "../include/Client.hpp"
#include <unistd.h>

using namespace std;

// Constructor
Client::Client(string client_id, int clientSocketFD, chrono::duration<double> keep_alive) 
  : client_id(client_id), keep_alive(keep_alive), last_seen(chrono::system_clock::now())
  , status(ClientStatus::CONNECTED), clientSocketFD(clientSocketFD), subscriptions(set<string>())
{  
  // no code needed here
}

// Destructor
Client::~Client() 
{
  close(this->clientSocketFD);
}

// convert the client to a string
string Client::to_string() 
{
  return this->client_id;
}

// set the client socket file descriptor
void Client::setClientSocketFD(int clientSocketFD) 
{
  this->clientSocketFD = clientSocketFD;
}

// get the client socket file descriptor
int Client::getClientSocketFD() 
{
  return this->clientSocketFD;
}

// set the keep alive duration
void Client::setKeepAlive(chrono::duration<double> keep_alive) 
{
  this->keep_alive = keep_alive;
}

// get the keep alive duration
chrono::duration<double> Client::getKeepAlive() 
{
  return this->keep_alive;
}

// update the last seen seen
void Client::updateLastSeen() 
{
  this->last_seen = chrono::system_clock::now();
}

// get the last seen seen
chrono::time_point<chrono::system_clock> Client::getLastSeen() 
{
  return this->last_seen;
}

// set the client status
void Client::setStatus(ClientStatus status) 
{
  this->status = status;
}

// get the client status
ClientStatus Client::getStatus() 
{
  return this->status;
}

// check if the client is alive
bool Client::isAlive() 
{
  chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
  chrono::duration<double> diff = now - this->last_seen;
  return diff < this->keep_alive;
}

// add a subscription to the client
void Client::addSubscription(const string& topic) 
{
  lock_guard<mutex> lock(subscriptionMutex);
  this->subscriptions.insert(topic);
}

// remove a subscription from the client
void Client::removeSubscription(const string& topic) 
{
  lock_guard<mutex> lock(subscriptionMutex);
  this->subscriptions.erase(topic);
}

// check if the client has a subscription
bool Client::hasSubscription(const string& topic) const 
{
  lock_guard<mutex> lock(subscriptionMutex);
  return this->subscriptions.find(topic) != this->subscriptions.end();
}

// get the client subscriptions
set<string> Client::getSubscriptions() const 
{
  lock_guard<mutex> lock(subscriptionMutex);
  return this->subscriptions;
}

// close the client file descriptor
void Client::closeFileDescriptor() 
{
  close(this->clientSocketFD);
}
