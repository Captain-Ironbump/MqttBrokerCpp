#pragma once

#include <cstdint>
#include <functional>  // For std::hash

class UUID 
{
public:
  UUID();
  UUID(uint64_t uuid);
  UUID(const UUID&) = default;
  uint64_t getUUID() const;
  bool operator==(const UUID& other) const 
  {
    return m_UUID == other.m_UUID;
  }
private:
  uint64_t m_UUID;
};

namespace std 
{
  template <>
  struct hash<UUID> 
  {
    std::size_t operator()(const UUID& uuid) const 
    {
      return hash<uint64_t>()(uuid.getUUID());
    }
  };
}
