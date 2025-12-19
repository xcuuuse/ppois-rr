#pragma once
#include <sc-memory/sc_memory.hpp>
struct Edge{
public:
  ScAddr street;
  ScAddr intersection;
  bool isUsed;
  bool operator == (Edge const & other) const
  {
    return street == other.street && intersection == other.intersection;
  }

};