#pragma once
#include <sc-memory/sc_memory.hpp>
struct Edge{
public:
  ScAddr street;
  bool visited;
  ScAddr intersection;
  bool operator == (Edge const & other) const
  {
    return street == other.street && intersection == other.intersection;
  }

};