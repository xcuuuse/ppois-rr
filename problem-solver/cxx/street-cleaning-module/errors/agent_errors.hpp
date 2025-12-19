#pragma once

#include <exception>
#include <string>

class StartNodeNotFoundError : public std::exception
{
private:
  std::string msg;

public:
  explicit StartNodeNotFoundError(std::string const & msg) {}

  char const * what()
  {
    return msg.c_str();
  }
};

class VertexError : public std::exception
{
private:
  std::string msg;

public:
  explicit VertexError(std::string const & msg) {}

  char const * what()
  {
    return msg.c_str();
  }
};

class NodeIsNotStreetError : public std::exception
{
private:
  std::string msg;

public:
  explicit NodeIsNotStreetError(std::string const & msg) {}

  char const * what()
  {
    return msg.c_str();
  }
};