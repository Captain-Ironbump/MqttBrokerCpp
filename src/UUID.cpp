#include "../include/UUID.hpp"

#include <random>

static std::random_device s_RandomDevice;
static std::mt19937_64 s_Engine(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_Distribution;

UUID::UUID() : m_UUID(s_Distribution(s_Engine)) 
{
}

UUID::UUID(uint64_t uuid) : m_UUID(uuid) 
{
}

uint64_t UUID::getUUID() const 
{
  return m_UUID;
}
  
