#pragma once
#include <sc-memory/sc_memory.hpp>
#include <string>
#include <vector>

enum class Euler
{
  None,
  Cycle
};

class Edge{
public:
  ScAddr street;
  bool visited;
  ScAddr intersection;
  bool operator == (Edge const & other) const
  {
    return street == other.street && intersection == other.intersection;
  }

};

class Route{
  double distance;
  ScAddrList edges;
};